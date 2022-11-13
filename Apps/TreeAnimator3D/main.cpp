#include "ImGuiManager.h"
#include "ErrorManager.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "DrawList.h"
#include "Shape.h"
#include "Camera2D.h"

#include <nothings-stb/stb_image.h>
#include <dearimgui/imgui.h>
#include "Angel-maths/mat.h"
#include <glew.h>
#include "Input.h"
#include <glfw3.h>
#include <cstdio>
#include <iostream>
#include <string>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GlFW Error %d: %s\n", error, description);
}


int main(int, char**)
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return -1;

	// Set up core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create GLFW full screen window
	GLFWmonitor* main_monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(main_monitor);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "CS 465 - Assignment 2 - Hierarchical Modeling", nullptr, nullptr);
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (!window || !mode)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwMaximizeWindow(window);
	glfwSwapInterval(1); // Enable vsync

	// Init GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Could not init GLEW..." << std::endl;
		GENERAL_BREAK();
	}
	else
	{
		const unsigned char* opengl_version;
		__glCallReturn(glGetString(GL_VERSION), opengl_version);
		std::cout << "OpenGL version: " << opengl_version << std::endl;

		const unsigned char* glsl_version;
		__glCallReturn(glGetString(GL_SHADING_LANGUAGE_VERSION), glsl_version);
		std::cout << "GLSL version: " << glsl_version << std::endl;
	}

	Input& window_input = Input::get_instance(window);

	// Setup Dear ImGui context
	init_imgui(window);
	SetupImGuiStyle();
	// ImGui state
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	float init_shape_length = width / 8.0f;

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// Line width for GL_LINES
	__glCallVoid(glLineWidth(3.0f));
	Angel::vec4* color_a = new Angel::vec4{ 0.6f, 0.9f, 0.0f, 1.0f };

	// Also initializes the basic shader
	Shape::init_static_members();

	// Texture Shader

	// Texture Slot 0 - Tree Surface
	Texture* tree_surface_texture_obj = new Texture("../../Data/textures/tree_surface_4k.png");
	tree_surface_texture_obj->bind(0);

	// Texture Slot 1 - Leafs
	Texture* leaf_texture_obj = new Texture("../../Data/textures/leaf.png");
	leaf_texture_obj->bind(1);

	// Use tree wood unless specified
	Shape::textured_shader()->bind();
	Shape::textured_shader()->set_uniform_1i("u_texture", 0);

	// Model a
	auto* model_a_pos = new Angel::vec3(width/2.0f, height/2.0f, 0.0f);
	auto* model_a_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	auto* model_a_scale = new Angel::vec3(width, height, 1.0f);
	ShapeModel* model_a = new ShapeModel(ShapeModel::StaticShape::RECTANGLE,
		model_a_pos,
		model_a_rot,
		model_a_scale,
		color_a
	);

	// Cube a
	Angel::vec3* cube_a_pos, * cube_a_rot, * cube_a_scale;
	cube_a_pos = new Angel::vec3(width/2.0f, height/2.0f, 0.0f);
	cube_a_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	cube_a_scale = new Angel::vec3(100.0f, 100.0f, 100.0f);
	ShapeModel* cube_a = new ShapeModel(ShapeModel::StaticShape::CUBE,
		cube_a_pos, cube_a_rot, cube_a_scale);

	Renderer renderer;

	// View matrix - camera
	float global_z_pos_2d = 0.0f;
	Camera2D::init(Angel::vec3(0.0f, 0.0f, global_z_pos_2d), 100.0f);
	auto view_matrix = Camera2D::view_matrix();

	// Orthographic projection is used
	Angel::mat4 projection_matrix = Angel::Ortho(0.0f, (float)width, (float)height, 0.0f, -1000.0f, 1000.0f);

	// Mouse location
	auto cursor_model_coords = Camera2D::map_from_global(0, 0);

	// TODO initialize platform surface cube

	DrawList list(projection_matrix, view_matrix);
	list.add_shape(model_a);
	list.add_shape(cube_a);

	bool is_dragging = false;
	bool should_update_sheet = true;
	const float& imgui_zoom_ratio = Camera2D::get_zoom_ratio();
	ImGuiColorEditFlags f = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;
	Angel::vec3 camera_pos_released;
	float camera_zoom_released;
	Angel::vec3 camera_pos_pressed;
	float camera_zoom_pressed;
	auto map_from_global_any = [](double x, double y, Angel::vec3 c_pos, float c_z) -> Angel::vec3
	{
		return (c_pos + Angel::vec3((float)x, (float)y, 0.0f)) * (100.0f / c_z);
	};
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Old mouse pos & state
		Angel::vec2 old_mouse_pos(window_input.m_mouse_x, window_input.m_mouse_y);
		Input::ButtonState mouse_previous_state = window_input.m_lmb_state;

		// Needed for selection and drawing while moving the camera
		const Camera2D& old_camera = Camera2D::get_instance();
		const Angel::vec3 old_camera_pos = old_camera.camera_pos();
		const float old_camera_zoom_ratio = old_camera.get_zoom_ratio();
		auto map_from_global_using_old_camera = [&, old_camera_pos, old_camera_zoom_ratio](double x, double y) -> Angel::vec3
		{
			return map_from_global_any(x, y, old_camera_pos, old_camera_zoom_ratio);
		};

		// Reset scroll
		window_input.m_scroll_y = 0.0;

		// Reset copy & paste
		window_input.m_copy_just_pressed = false;
		window_input.m_paste_just_pressed = false;

		glfwPollEvents();

		// Update the viewport
		glfwGetWindowSize(window, &width, &height);

		// Update the window projection
		projection_matrix = Angel::Ortho(0.0f, (float)width, (float)height, 0.0f, -1000.0f, 1000.0f);

		// Update cursor
		cursor_model_coords = Camera2D::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
		bool input_on_imgui = ImGui::GetIO().WantCaptureMouse;

		// Keyboard & Mouse Wheel Events
		{
			if (!input_on_imgui)
			{
				// Continuous key presses with getKey commands
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				{
					Camera2D::move_vertical(-20.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				{
					Camera2D::move_horizontal(-20.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				{
					Camera2D::move_vertical(20.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				{
					Camera2D::move_horizontal(20.0f);
				}

				// Camera zooming with mouse wheel
				Camera2D::zoom(window_input.m_scroll_y, window_input.m_mouse_x, window_input.m_mouse_y);

				// Update view matrix when necessary
				view_matrix = Camera2D::view_matrix();
				// Immediate events coming from callbacks
				if (window_input.m_copy_just_pressed)
				{
					// Copy command sent
				}
				if (window_input.m_paste_just_pressed)
				{
					// Paste command sent
				}
			}
		}

		// LMB States
		if (mouse_previous_state == Input::ButtonState::BeingPressed
			&& window_input.m_lmb_state == Input::ButtonState::Released)
		{
			camera_pos_released = Camera2D::camera_pos();
			camera_zoom_released = Camera2D::get_zoom_ratio();
			if (!input_on_imgui)
			{
				// Finish dragging
				if (is_dragging)
				{
					Angel::vec3 cursor_released = map_from_global_any(window_input.m_mouse_release_x, window_input.m_mouse_release_y, camera_pos_released, camera_zoom_released);
					if (std::abs(window_input.m_mouse_release_y - window_input.m_mouse_press_y) < 1.0f
						&& std::abs(window_input.m_mouse_release_x - window_input.m_mouse_press_x) < 1.0f)
					{
						std::cout << "THIS IS A CLICK" << std::endl;

						// Do sth. with the click - cursor_released vector is the world coordinate of the click

					}
					else
					{
						std::cout << "THIS IS A DRAG" << std::endl;

						Angel::vec3 cursor_pressed = map_from_global_any(window_input.m_mouse_press_x, window_input.m_mouse_press_y, camera_pos_pressed, camera_zoom_pressed);
						Angel::vec3 drag_vector = cursor_released - cursor_pressed;

						// Do sth. with drag vector here
						std::cout << "Total drag amount: " << std::to_string(drag_vector.x) << ", "
							<< std::to_string(drag_vector.y) << ")" << std::endl;
					}
				}
			}
			else
			{
				// Application specific state consumptions when ImGui has focus in mouse release
			}

			// Consume the released state
			window_input.m_mouse_press_y = -1.0f;
			window_input.m_mouse_press_x = -1.0f;
			window_input.m_lmb_state = Input::ButtonState::Idle;
			std::cout << "LMB is now Idle" << std::endl;
			is_dragging = false;
			camera_pos_pressed = Angel::vec3(0.0f);
			camera_zoom_pressed = 0.0f;
		}
		else if (mouse_previous_state == Input::ButtonState::Idle
			&& window_input.m_lmb_state == Input::ButtonState::JustPressed)
		{

			camera_pos_pressed = Camera2D::camera_pos();
			camera_zoom_pressed = Camera2D::get_zoom_ratio();
			if (!input_on_imgui)
			{
				// Scene event handling when the LMB is just pressed
				is_dragging = true;
				Angel::vec3 mouse_model_old = map_from_global_any(window_input.m_mouse_press_x, window_input.m_mouse_press_y, camera_pos_pressed, camera_zoom_pressed);
				Angel::vec3 mouse_model_new = Camera2D::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
				Angel::vec3 mouse_drag_rect_scale = (1.0f) * (mouse_model_new - mouse_model_old);
				if (mouse_drag_rect_scale.x == 0.0f)
				{
					mouse_drag_rect_scale.x = 1.0f;
				}
				if (mouse_drag_rect_scale.y == 0.0f)
				{
					mouse_drag_rect_scale.y = 1.0f;
				}
				mouse_drag_rect_scale.z = 1.0f;
			}

			// Consume the pressed state
			window_input.m_lmb_state = Input::ButtonState::BeingPressed;
			std::cout << "LMB is now BeingPressed" << std::endl;
			window_input.m_mouse_release_y = -1.0f;
			window_input.m_mouse_release_x = -1.0f;
			camera_pos_released = {};
			camera_zoom_released = {};
		}
		else if (mouse_previous_state == Input::ButtonState::BeingPressed)
		{
			if (!input_on_imgui)
			{
				// Scene event handling when the mouse is being pressed
				Angel::vec3 mouse_model_old = map_from_global_any(window_input.m_mouse_press_x, window_input.m_mouse_press_y, camera_pos_pressed, camera_zoom_pressed);
				Angel::vec3 mouse_model_new = Camera2D::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
				Angel::vec3 mouse_drag_rect_scale = (1.0f) * (mouse_model_new - mouse_model_old);
				if (mouse_drag_rect_scale.x == 0.0f)
				{
					mouse_drag_rect_scale.x = 1.0f;
				}
				if (mouse_drag_rect_scale.y == 0.0f)
				{
					mouse_drag_rect_scale.y = 1.0f;
				}
				mouse_drag_rect_scale.z = 1.0f;
			}
		}
		else if (window_input.m_lmb_state == Input::ButtonState::Idle)
		{
			// Optional mouse  event handling in idle state, e.g. drawing a line from
			// a point chosen before towards the current mouse coordinate
		}

		// RMB States
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			// Event handling as long as RMB is pressed
			// This is equivalent to a (JustPressed | BeingPressed) state
		}

		// ImGui Components 
		new_imgui_frame();
		{

			ImGui::Begin("Hello, world!");
			{
				ImGui::Text("This is some useful text.");
				ImGui::NewLine();

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
					1000.0f / ImGui::GetIO().Framerate,
					ImGui::GetIO().Framerate);

				// Get cursor model coordinates
				ImGui::Text("Cursor Model Coordinates: %f, %f", cursor_model_coords.x, cursor_model_coords.y);
				ImGui::End();
			}

			ImGui::EndFrame();
		}

		// Clear background
		Renderer::set_viewport(window);
		Renderer::clear((float*)&clear_color);

		// Draw the draw list
		list.draw_all();

		// Always draw ImGui on top of the app
		render_imgui();

		glfwSwapBuffers(window);
	}

	// Clear the draw list & delete the VB/IB/VA objects for polygons
	list.shutdown();

	// Cleanup
	Shape::destroy_static_members_allocated_on_the_heap();

	// Shutdown ImGui & GLFW
	shutdown_imgui();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
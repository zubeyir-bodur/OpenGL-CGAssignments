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
	ImVec4 clear_color = ImVec4(0.3984375f, 0.3984375f, 0.3984375f, 1.0f);
	float init_shape_length = width / 8.0f;

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// Line width for GL_LINES
	__glCallVoid(glLineWidth(5.0f));

	// Specify the color of the triangle
	float color_sheet[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	Angel::vec4* color_a = new Angel::vec4{ 0.6f, 0.9f, 0.0f, 1.0f };
	Angel::vec4* color_b = new Angel::vec4{ 0.9f, 0.6f, 0.0f, 1.0f };
	Angel::vec4* color_c = new Angel::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };
	Angel::vec4* color_d = new Angel::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };

	Shape::init_static_members(width);
	// Texture
	Texture* texture_obj;
#define has_texture false
	auto* shader_texture = new Shader("../../Engine/shaders/textured_triangle.glsl");;
	if (has_texture)
	{
		texture_obj = new Texture("../../Data/textures/eye.png");
		texture_obj->bind();
		shader_texture->set_uniform_1i("u_texture", 0);
	}
	Renderer renderer;
	float imgui_height = 0.0f;
	Angel::vec3 sheet_pos(0, imgui_height, 0.0f);

	// Initialize shapes
	// positions - respect to their initial 0th vertex positions
	auto* model_a_pos = new Angel::vec3(0, 0.0f, 0.0f);
	auto* model_b_pos = new Angel::vec3(width / 2.0f - init_shape_length / 2, height / 2.0f - init_shape_length / 2, 0.0f);
	auto* model_c_pos = new Angel::vec3(width / 4.0f - init_shape_length / 2, height / 4.0f - init_shape_length / 2, 0.0f);

	// rotation - in radians (x, y, z axises respectively)
	auto* model_a_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	auto* model_b_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	auto* model_c_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	// scale   
	auto* model_a_scale = new Angel::vec3(320.0f, 320.0f, 1.0f);
	auto* model_b_scale = new Angel::vec3(320.0f, 320.0f, 1.0f);
	auto* model_c_scale = new Angel::vec3(320.0f, 320.0f, 1.0f);

	ShapeModel* model_a = new ShapeModel(ShapeModel::StaticShape::RECTANGLE,
		model_a_pos,
		model_a_rot,
		model_a_scale,
		color_a
	);
	ShapeModel* model_b = new ShapeModel(ShapeModel::StaticShape::RECTANGLE,
		model_b_pos,
		model_b_rot,
		model_b_scale,
		color_b
	);
	ShapeModel* model_c = new ShapeModel(ShapeModel::StaticShape::ISOSCELES_TRIANGLE,
		model_c_pos,
		model_c_rot,
		model_c_scale,
		color_c
	);

	// Test for polygon creation
	constexpr float global_z_pos_2d = 0.0f;
	std::vector<Angel::vec3> poly_coords{
	Angel::vec3(-init_shape_length / 2,	init_shape_length / 2,	global_z_pos_2d),	// 0
	Angel::vec3(init_shape_length / 2,	init_shape_length / 2,	global_z_pos_2d),	// 1
	Angel::vec3(init_shape_length,		0.0f,					global_z_pos_2d),	// 2
	Angel::vec3(init_shape_length / 2,	-init_shape_length / 2,	global_z_pos_2d),	// 3
	Angel::vec3(-init_shape_length / 2,	-init_shape_length / 2,	global_z_pos_2d),	// 4
	Angel::vec3(-init_shape_length,		0.0f,					global_z_pos_2d),	// 5
	};

	auto* model_d = new ShapeModel(poly_coords,
		color_d
	);

	// Tests for adding a vertex provided that concaveness remains
	model_d->push_back_vertex(Angel::vec3(-3 * init_shape_length / 3.0f, 3 * init_shape_length / 7.0f, global_z_pos_2d));

	// View matrix - camera
	Camera2D::init(Angel::vec3(0.0f, 0.0f, global_z_pos_2d), 100.0f);
	auto view_matrix = Camera2D::view_matrix();

	// Orthographic projection is used
	Angel::mat4 projection_matrix = Angel::Ortho2D(0.0f, (float)width, (float)height, 0.0f);

	// Mouse location
	auto cursor_model_coords = Camera2D::map_from_global(0, 0);

	// Sheet initializations
	Angel::mat4 model_sheet_matrix = Angel::Translate(sheet_pos)
		* Angel::Scale(Angel::vec3(width, height, 1.0f));
	Angel::mat4 MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;

	DrawList list(projection_matrix, view_matrix);
	list.add_shape(model_a);
	list.add_shape(model_b);
	list.add_shape(model_c);
	list.add_shape(model_d);
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

		if (should_update_sheet)
		{
			sheet_pos = Angel::vec3(0, imgui_height, 0.0f);
			model_sheet_matrix = Angel::Translate(sheet_pos)
				* Angel::Scale(Angel::vec3(width, height, 1.0f));
			should_update_sheet = false;
		}

		// Update the window projection
		projection_matrix = Angel::Ortho2D(0.0f, (float)width, (float)height, 0.0f);

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

						// Do sth with the click - cursor_released vector is the world coordinate of the click

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

		// update center positions of models for logging
		Angel::vec3 size_a = model_a->shape_size();
		Angel::vec3 center_a = model_a->center_true();
		Angel::vec3 size_b = model_b->shape_size();
		Angel::vec3 center_b = model_b->center_true();
		Angel::vec3 size_c = model_c->shape_size();
		Angel::vec3 center_c = model_c->center_true();
		Angel::vec3 size_d = model_d->shape_size();
		Angel::vec3 center_d = model_d->center_true();

		// ImGui Components 
		new_imgui_frame();
		{

			ImGui::Begin("Hello, world!");
			{
				ImGui::Text("This is some useful text.");

				ImGui::SliderFloat("Model A-Xpos", &model_a_pos->x, 0.0f, (float)mode->width, "%.1f", 1.0f);
				ImGui::SliderFloat("Model A-Ypos", &model_a_pos->y, 0.0f, (float)mode->height, "%.1f", 1.0f);
				ImGui::SliderFloat("Model A-zrot", &model_a_rot->z, 0.0f, 360, "%.3f", 1.0f);
				ImGui::SliderFloat2("Model A-scale", &model_a_scale->x, -init_shape_length, init_shape_length, "%.3f", 1.0f);
				ImGui::Text("Size of Model A: %f, %f", size_a.x, size_a.y);
				ImGui::Text("Position of the Center of Model A: %f, %f", center_a.x, center_a.y);
				ImGui::NewLine();

				ImGui::SliderFloat("Model B-XPos", &model_b_pos->x, 0.0f, (float)mode->width, "%.1f", 1.0f);
				ImGui::SliderFloat("Model B-YPos", &model_b_pos->y, 0.0f, (float)mode->height, "%.1f", 1.0f);
				ImGui::SliderFloat("Model B-zrot", &model_b_rot->z, 0.0f, 360, "%.3f", 1.0f);
				ImGui::SliderFloat2("Model B-scale", &model_b_scale->x, -init_shape_length, init_shape_length, "%.3f", 1.0f);
				ImGui::Text("Size of Model A: %f, %f", size_b.x, size_b.y);
				ImGui::Text("Position of the Center of Model B: %f, %f", center_b.x, center_b.y);
				ImGui::NewLine();

				ImGui::SliderFloat("Model C-XPos", &model_c_pos->x, 0.0f, (float)mode->width, "%.1f", 1.0f);
				ImGui::SliderFloat("Model C-YPos", &model_c_pos->y, 0.0f, (float)mode->height, "%.1f", 1.0f);
				ImGui::SliderFloat("Model C-zrot", &model_c_rot->z, 0.0f, 360, "%.3f", 1.0f);
				ImGui::SliderFloat2("Model C-scale", &model_c_scale->x, -init_shape_length, init_shape_length, "%.3f", 1.0f);
				ImGui::Text("Size of Model C: %f, %f", size_c.x, size_c.y);
				ImGui::Text("Position of the Center of Model C: %f, %f", center_c.x, center_c.y);
				ImGui::NewLine();

				ImGui::SliderFloat("Model D-XPos", &model_d->position().x, -500.0f, (float)mode->width, "%.1f", 1.0f);
				ImGui::SliderFloat("Model D-YPos", &model_d->position().y, -500.0f, (float)mode->height, "%.1f", 1.0f);
				ImGui::SliderFloat("Model D-zrot", &model_d->rotation().z, 0.0f, 360, "%.3f", 1.0f);
				ImGui::Text("Size of Model D: %f, %f", size_d.x, size_d.y);
				ImGui::Text("Position of the Center of Model D: %f, %f", center_d.x, center_d.y);
				ImGui::NewLine();

				if (!has_texture)
				{
					ImGui::ColorEdit4("Model A Color", &color_a->x, f);
					ImGui::SameLine();
					ImGui::ColorEdit4("Model B Color", &color_b->x, f);
					ImGui::SameLine();
					ImGui::ColorEdit4("Model C Color", &color_c->x, f);
					ImGui::SameLine();
					ImGui::ColorEdit4("Model D Color", &color_d->x, f);
					ImGui::NewLine();
				}

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

		// Shader for sheet
		Shape::shader()->bind();
		Shape::shader()->set_uniform_4f("u_color",
			color_sheet[0],
			color_sheet[1],
			color_sheet[2],
			color_sheet[3]);
		view_matrix = Camera2D::view_matrix();
		MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;
		Shape::shader()->set_uniform_mat4f("u_MVP", MVP_mat_sheet);

		// Draw the sheet
		Renderer::draw_triangles(Shape::rectangle()->vertex_array(), Shape::rectangle()->triangles_index_buffer(), Shape::shader());

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
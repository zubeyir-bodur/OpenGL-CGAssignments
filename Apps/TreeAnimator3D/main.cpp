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
#include "PerspectiveCamera.h"

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

	Renderer renderer;

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	renderer.clear(&clear_color.x);

	// Enable Depth Test & Face Culling
	__glCallVoid(glEnable(GL_DEPTH_TEST));
	__glCallVoid(glDepthMask(GL_TRUE));
	__glCallVoid(glEnable(GL_CULL_FACE));
	__glCallVoid(glFrontFace(GL_CCW));
	__glCallVoid(glCullFace(GL_BACK));

	// Line width for GL_LINES
	__glCallVoid(glLineWidth(1.0f));
	Angel::vec4* color_a = new Angel::vec4{ 0.6f, 0.0f, 0.0f, 1.0f };

	// Also initializes the basic shader
	Shape::init_static_members();

	// Texture Shader

	// Texture Slot 0 - Tree Surface
	Texture* tree_surface_texture_obj = new Texture("../../Data/textures/tree_surface_4k.png");
	tree_surface_texture_obj->bind(0);

	// Texture Slot 1 - Leafs
	Texture* leaf_texture_obj = new Texture("../../Data/textures/leaf.png");
	leaf_texture_obj->bind(1);
	tree_surface_texture_obj->unbind();
	tree_surface_texture_obj->unbind();
	// Platform surface
	Angel::vec3* platform_surface_pos, * platform_surface_rot, * platform_surface_scale;
	platform_surface_pos = new Angel::vec3(0.0f, -300.0f, -2000.0f);
	platform_surface_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	platform_surface_scale = new Angel::vec3((float)width, 20.0f, (float)height);
	ShapeModel* platform_surface = new ShapeModel(ShapeModel::StaticShape::COL_CUBE,
		platform_surface_pos, platform_surface_rot, platform_surface_scale);

	// A textured cube
	Angel::vec3* text_a_pos, * text_a_rot, * text_a_scale;
	text_a_pos = new Angel::vec3(0.0f, 0.0f, -400.0f);
	text_a_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	text_a_scale = new Angel::vec3(100, 100.0f, 100);
	ShapeModel* text_a = new ShapeModel(ShapeModel::StaticShape::TEX_CUBE,
		text_a_pos,
		text_a_rot,
		text_a_scale,
		0,
		tree_surface_texture_obj);

	// View matrix - camera
	PerspectiveCamera::init(Angel::vec3(0.0f, 0.0f, 0.0f), 60.0f, (float)width / (float)height);
	const Angel::mat4& view_matrix = PerspectiveCamera::view_matrix();
	const Angel::mat4& proj_matrix = PerspectiveCamera::proj_matrix();

	// Draw List
	DrawList list(proj_matrix, view_matrix);
	list.add_shape(platform_surface);
	list.add_shape(text_a);

	bool is_dragging = false;
	bool should_update_sheet = true;
	ImGuiColorEditFlags f = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;
	Angel::vec3 camera_pos_released;
	Angel::vec3 camera_pos_pressed;
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Old mouse pos & state
		Angel::vec2 old_mouse_pos((float)window_input.m_mouse_x, (float)window_input.m_mouse_y);
		Input::ButtonState mouse_previous_state = window_input.m_lmb_state;

		// Needed for selection and drawing while moving the camera
		const PerspectiveCamera& old_camera = PerspectiveCamera::get_instance();
		const Angel::vec3 old_camera_pos = old_camera.position();

		// Reset scroll
		window_input.m_scroll_y = 0.0;

		// Reset copy & paste
		window_input.m_copy_just_pressed = false;
		window_input.m_paste_just_pressed = false;

		glfwPollEvents();

		// Update the viewport
		glfwGetWindowSize(window, &width, &height);

		// Update the window projection
		PerspectiveCamera::resize((float)width/(float)height);

		// Update cursor
		bool input_on_imgui = ImGui::GetIO().WantCaptureMouse;

		// Keyboard & Mouse Wheel Events
		{
			if (!input_on_imgui)
			{
				// Continuous key presses with getKey commands
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				{
					PerspectiveCamera::dolly(-1.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				{
					PerspectiveCamera::truck(-1.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				{
					PerspectiveCamera::dolly(1.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				{
					PerspectiveCamera::truck(1.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				{
					PerspectiveCamera::pedestal(-1.0f);
				}
				if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
				{
					PerspectiveCamera::pedestal(1.0f);
				}


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
			camera_pos_released = PerspectiveCamera::position();
			if (!input_on_imgui)
			{
				// Finish dragging
				if (is_dragging)
				{
					if (std::abs(window_input.m_mouse_release_y - window_input.m_mouse_press_y) < 1.0f
						&& std::abs(window_input.m_mouse_release_x - window_input.m_mouse_press_x) < 1.0f)
					{
						std::cout << "THIS IS A CLICK" << std::endl;

						// Do sth. with the click - cursor_released vector is the world coordinate of the click

					}
					else
					{
						std::cout << "THIS IS A DRAG" << std::endl;
						
						// Do sth. with drag vector here

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
		}
		else if (mouse_previous_state == Input::ButtonState::Idle
			&& window_input.m_lmb_state == Input::ButtonState::JustPressed)
		{

			camera_pos_pressed = PerspectiveCamera::position();
			if (!input_on_imgui)
			{
				// Scene event handling when the LMB is just pressed
				is_dragging = true;
			}

			// Consume the pressed state
			window_input.m_lmb_state = Input::ButtonState::BeingPressed;
			std::cout << "LMB is now BeingPressed" << std::endl;
			window_input.m_mouse_release_y = -1.0f;
			window_input.m_mouse_release_x = -1.0f;
			camera_pos_released = {};
		}
		else if (mouse_previous_state == Input::ButtonState::BeingPressed)
		{
			if (!input_on_imgui)
			{
				PerspectiveCamera::tilt(((float)window_input.m_mouse_x - (float)old_mouse_pos.x) / 10.0f);
				PerspectiveCamera::pan(((float)window_input.m_mouse_y - (float)old_mouse_pos.y) / 10.0f);
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

// 				ImGui::SliderFloat("Platform-XPos", &platform_surface->position().x, -500.0f, (float)mode->width, "%.1f", 1.0f);
// 				ImGui::SliderFloat("Platform-YPos", &platform_surface->position().y, -500.0f, (float)mode->height, "%.1f", 1.0f);
// 				ImGui::SliderFloat("Platform-zPos", &platform_surface->position().z, -500.0f, (float)mode->height, "%.1f", 1.0f);
// 				ImGui::SliderFloat("Platform-xrot", &platform_surface->rotation().x, 0.0f, 360, "%.3f", 1.0f);
// 				ImGui::SliderFloat("Platform-yrot", &platform_surface->rotation().y, 0.0f, 360, "%.3f", 1.0f);
// 				ImGui::SliderFloat("Platform-zrot", &platform_surface->rotation().z, 0.0f, 360, "%.3f", 1.0f);


				ImGui::NewLine();

				ImGui::SliderFloat("Crate-XPos", &text_a->position().x, -500.0f, (float)mode->width, "%.1f", 1.0f);
				ImGui::SliderFloat("Crate-YPos", &text_a->position().y, -500.0f, (float)mode->height, "%.1f", 1.0f);
				ImGui::SliderFloat("Crate-zPos", &text_a->position().z, -500.0f, (float)mode->height, "%.1f", 1.0f);
				ImGui::SliderFloat("Crate-xrot", &text_a->rotation().x, -180.0f, 180.0f,  "%.3f", 1.0f);
				ImGui::SliderFloat("Crate-yrot", &text_a->rotation().y, -180.0f, 180.0f,  "%.3f", 1.0f);
				ImGui::SliderFloat("Crate-zrot", &text_a->rotation().z, -180.0f, 180.0f,  "%.3f", 1.0f);
				ImGui::SliderFloat("Crate-xscale", &text_a->scale().x, 0.0f, 200, "%.3f", 1.0f);
				ImGui::SliderFloat("Crate-yscale", &text_a->scale().y, 0.0f, 200, "%.3f", 1.0f);
				ImGui::SliderFloat("Crate-zscale", &text_a->scale().z, 0.0f, 200, "%.3f", 1.0f);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
					1000.0f / ImGui::GetIO().Framerate,
					ImGui::GetIO().Framerate);

				// Get cursor model coordinates
				ImGui::End();
			}

			ImGui::EndFrame();
		}
		// Update ViewProj matrix
		PerspectiveCamera::update();

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
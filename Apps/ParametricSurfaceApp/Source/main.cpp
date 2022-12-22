#include "Core/ImGuiManager.h"
#include "Core/ErrorManager.h"

#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexBufferLayout.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"

#include "EntityManager/Shape.h"
#include "EntityManager/ParametricMesh.h"

#include "Camera/PerspectiveCamera.h"

#include <dearimgui/imgui.h>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "Angel-maths/mat.h"
#include <glew.h>

#include "Platform/Input.h"
#include <glfw3.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>
#include <magic_enum/magic_enum.hpp>

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
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "CS 465 - Assignment 3 - Realistic Rendering on Parametric Surfaces", nullptr, nullptr);
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (!window || !mode)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwMaximizeWindow(window);
	glfwSwapInterval(0); // Disable vsync

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

	// ImGui & app state
	bool is_dragging = false;
	ImVec2 editor_pane_size(width / 5.0f, (float)height);
	ImGuiColorEditFlags color_edit_flags = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;
	ImGuiWindowFlags flags_editor_pane =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoResize;
	float init_shape_length = width / 8.0f;
	ImVec4 clear_col = { 0.6f, 0.6f, 0.6f, 1.0f };
	Renderer renderer;
	int radio_button_cur = (int)ParametricMesh::DisplayType::Wireframe;

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	renderer.clear(&clear_col.x);

	// Enable Depth Test for displaying inside of the objects as well
	__glCallVoid(glEnable(GL_DEPTH_TEST));
	__glCallVoid(glDepthFunc(GL_LEQUAL));
	__glCallVoid(glEnable(GL_POLYGON_OFFSET_FILL));
	__glCallVoid(glPolygonOffset(1.0f, 2.0f));

	// Line width for GL_LINES
	__glCallVoid(glLineWidth(1.0f));

	// Also initializes the basic shader
	Shape::init_static_members();

	// Surface parameters & mesh
	float R = 1.5f;
	float r = 1.0f;
	float l = 3.0f;
	float q = 30.0f;
	unsigned int rsbd = 20, csbd = 20;
	ParametricMesh::init_static_members();
	auto* bumpmap = new BumpMap("../../Data/maps/bump_example.png");
	ParametricMesh* four_i = new ParametricMesh(R, r, l, q, rsbd, csbd, {1.0f, 1.0f, 1.0f, 1.0f}, bumpmap);

	// View matrix - camera
	PerspectiveCamera::init({ 0.2f, 2.0f, -0.6f }, { -30.0f, -180.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 60.0f, width, height);
	PerspectiveCamera::set_speed(10.0f, 0.1f, 2500.0f);
	const Angel::mat4& view_matrix = PerspectiveCamera::view_matrix();
	const Angel::mat4& proj_matrix = PerspectiveCamera::projection_matrix();

	// Delta time recording
	auto last_frame_time = 0.0;

	// Rendering & Event Loop
	while (!glfwWindowShouldClose(window))
	{
		auto time = glfwGetTime();
		// Compute time between frames
		float delta_time_seconds = static_cast<float>(time - last_frame_time);
		last_frame_time = time;

		// Old mouse pos & state
		Angel::vec2 old_mouse_pos((float)window_input.m_mouse_x, (float)window_input.m_mouse_y);
		Input::ButtonState left_mouse_previous_state = window_input.m_lmb_state;
		Input::ButtonState right_mouse_previous_state = window_input.m_rmb_state;

		// Reset scroll
		window_input.m_scroll_y = 0.0;

		glfwPollEvents();

		// Update the viewport
		int old_width = width, old_height = height;
		glfwGetWindowSize(window, &width, &height);

		// Update the window projection
		if (old_width != width || old_height != height)
		{
			PerspectiveCamera::on_viewport_resize(width, height);
			editor_pane_size = { width / 5.0f, (float)height };
		}

		// Update cursor
		bool input_on_imgui = ImGui::GetIO().WantCaptureMouse;

		// Keyboard States
		{
			if (!input_on_imgui)
			{
				// Continuous key presses with getKey commands
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				{
					PerspectiveCamera::move(delta_time_seconds, PerspectiveCamera::MovementDirection::FORWARD);
				}
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				{
					PerspectiveCamera::move(delta_time_seconds, PerspectiveCamera::MovementDirection::BACKWARD);
				}
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				{
					PerspectiveCamera::move(delta_time_seconds, PerspectiveCamera::MovementDirection::LEFT);
				}
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				{
					PerspectiveCamera::move(delta_time_seconds, PerspectiveCamera::MovementDirection::RIGHT);
				}
				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				{
					PerspectiveCamera::move(delta_time_seconds, PerspectiveCamera::MovementDirection::DOWN);
				}
				if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
				{
					PerspectiveCamera::move(delta_time_seconds, PerspectiveCamera::MovementDirection::UP);
				}
			}
		}

		// Wheel States
		{
			PerspectiveCamera::zoom(delta_time_seconds, (float)window_input.m_scroll_y);
		}

		// LMB States
		{
			if (left_mouse_previous_state == Input::ButtonState::Released
				&& window_input.m_lmb_state == Input::ButtonState::Released)
			{
				window_input.m_mouse_press_y = -1.0f;
				window_input.m_mouse_press_x = -1.0f;
				window_input.m_mouse_release_y = -1.0f;
				window_input.m_mouse_release_x = -1.0f;
				window_input.m_lmb_state = Input::ButtonState::Idle;
				std::cout << "Bug-fix occurred" << std::endl;
				is_dragging = false;
			}

			if (left_mouse_previous_state == Input::ButtonState::BeingPressed
				&& window_input.m_lmb_state == Input::ButtonState::Released)
			{
				if (!input_on_imgui)
				{
					// Finish dragging
					if (is_dragging)
					{
						if (std::abs(window_input.m_mouse_release_y - window_input.m_mouse_press_y) < 1.0f
							&& std::abs(window_input.m_mouse_release_x - window_input.m_mouse_press_x) < 1.0f)
						{
							std::cout << "THIS IS A CLICK" << std::endl;
						}
						else
						{
							std::cout << "THIS IS A DRAG" << std::endl;
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
			}
			else if (left_mouse_previous_state == Input::ButtonState::Idle
				&& window_input.m_lmb_state == Input::ButtonState::JustPressed)
			{
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
			}
			else if (left_mouse_previous_state == Input::ButtonState::BeingPressed)
			{
				if (!input_on_imgui)
				{
				}
			}
			else if (window_input.m_lmb_state == Input::ButtonState::Idle)
			{
				// Optional mouse  event handling in idle state, e.g. drawing a line from
				// a point chosen before towards the current mouse coordinate
			}
		}

		// RMB States
		{
			if (right_mouse_previous_state == Input::ButtonState::BeingPressed
				&& window_input.m_rmb_state == Input::ButtonState::Released)
			{
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
				window_input.m_rmb_state = Input::ButtonState::Idle;
				std::cout << "RMB is now Idle" << std::endl;
				is_dragging = false;
			}
			else if (right_mouse_previous_state == Input::ButtonState::Idle
				&& window_input.m_rmb_state == Input::ButtonState::JustPressed)
			{
				if (!input_on_imgui)
				{
					PerspectiveCamera::rotate(delta_time_seconds,
						window_input.m_mouse_x - old_mouse_pos.x,
						window_input.m_mouse_y - old_mouse_pos.y);
					// Scene event handling when the LMB is just pressed
					is_dragging = true;
				}

				// Consume the pressed state
				window_input.m_rmb_state = Input::ButtonState::BeingPressed;
				std::cout << "RMB is now BeingPressed" << std::endl;
				window_input.m_mouse_release_y = -1.0f;
				window_input.m_mouse_release_x = -1.0f;
			}
			else if (right_mouse_previous_state == Input::ButtonState::BeingPressed)
			{
				if (!input_on_imgui)
				{
					PerspectiveCamera::rotate(delta_time_seconds,
						window_input.m_mouse_x - old_mouse_pos.x,
						window_input.m_mouse_y - old_mouse_pos.y);
				}
			}
			else if (window_input.m_rmb_state == Input::ButtonState::Idle)
			{
				// Optional mouse  event handling in idle state, e.g. drawing a line from
				// a point chosen before towards the current mouse coordinate
			}
		}

		// ImGui Components 
		new_imgui_frame();
		{
			ImGui::SetNextWindowPos(ImVec2(width - editor_pane_size.x, 0), ImGuiCond_::ImGuiCond_Always);
			ImGui::SetNextWindowSize(editor_pane_size, ImGuiCond_::ImGuiCond_Always);
			if (ImGui::Begin("Editor Pane", nullptr, flags_editor_pane))
			{
				if (ImGui::BeginTabBar("##tabs"))
				{
					if (ImGui::BeginTabItem("Model"))
					{
						ImGui::Text("Currently, a cephalopod is being displayed...");
						ImGui::NewLine();
						ImGui::RadioButton("Wireframe", &radio_button_cur, (int)ParametricMesh::DisplayType::Wireframe);
						ImGui::SameLine();
						ImGui::RadioButton("Gouraud", &radio_button_cur, (int)ParametricMesh::DisplayType::Gouraud);
						ImGui::SameLine();
						ImGui::RadioButton("Phong", &radio_button_cur, (int)ParametricMesh::DisplayType::Phong);
						ImGui::NewLine();
						ImGui::SliderFloat("R", &R, 0.1f, 30.0f, "%.3f", 1.0f);
						ImGui::SliderFloat("r", &r, 0.1f, 20.0f, "%.3f", 1.0f);
						ImGui::SliderFloat("l", &l, 0.1f, 4.0f, "%.3f", 1.0f);
						ImGui::SliderFloat("q", &q, 0.1f, 40.0f, "%.3f", 1.0f);
						ImGui::SliderInt("Row Subdiv.", (int*)&rsbd, 20, 200, "%d", 0);
						ImGui::SliderInt("Col Subdiv.", (int*)&csbd, 20, 200, "%d", 0);
						four_i->set_R(R);
						four_i->set_r(r);
						four_i->set_l(l);
						four_i->set_q(q);
						four_i->set_row_subdiv(rsbd);
						four_i->set_col_subdiv(csbd);
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Lighting & Shading"))
					{
						ImGui::ColorEdit4("Albedo color", &four_i->color().x, color_edit_flags);
						ImGui::ColorEdit4("Ambient color", &four_i->ambient().x, color_edit_flags);
						ImGui::ColorEdit4("Diffuse color", &four_i->diffuse().x, color_edit_flags);
						ImGui::ColorEdit4("Specular color", &four_i->specular().x, color_edit_flags);
						ImGui::SliderFloat("Shininess", &four_i->shininess(), 1.0f, 100.0f, "%.1f", 1.0f);
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Camera"))
					{
						ImGui::Text("Camera-Pos.x: %f", PerspectiveCamera::position().x);
						ImGui::Text("Camera-Pos.y: %f", PerspectiveCamera::position().y);
						ImGui::Text("Camera-Pos.z: %f", PerspectiveCamera::position().z);
						ImGui::Text("Pitch: %f", PerspectiveCamera::pitch());
						ImGui::Text("Yaw: %f", PerspectiveCamera::yaw());
						ImGui::Text("Roll: %f", PerspectiveCamera::roll());
						ImGui::Text("Zoom Ratio: %f", PerspectiveCamera::zoom_ratio());

						ImGui::NewLine();
						ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
							1000.0f / ImGui::GetIO().Framerate,
							ImGui::GetIO().Framerate);
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::End();
			}

		}
		ImGui::EndFrame();

		// Clear background
		Renderer::set_viewport(window);
		Renderer::clear(&clear_col.x);

		four_i->update_and_draw(proj_matrix, view_matrix, (ParametricMesh::DisplayType)radio_button_cur);

		// Always draw ImGui on top of the app
		render_imgui();

		glfwSwapBuffers(window);
	}

	// Cleanup
	Shape::destroy_static_members_allocated_on_the_heap();
	delete four_i;
	delete bumpmap;
	ParametricMesh::destroy_static_members();

	// Shutdown ImGui & GLFW
	shutdown_imgui();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
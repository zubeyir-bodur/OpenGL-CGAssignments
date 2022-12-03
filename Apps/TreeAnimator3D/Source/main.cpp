#include "Core/ImGuiManager.h"
#include "Core/ErrorManager.h"

#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexBufferLayout.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"

#include "EntityManager/DrawList.h"
#include "EntityManager/UndoRedoStack.h"
#include "EntityManager/Shape.h"
#include "EntityManager/DSerializer.h"
#include "EntityManager/SelectionSystem3D.h"
#include "EntityManager/ArticulatedModel.h"

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
	// ImGui state
	float init_shape_length = width / 8.0f;

	Renderer renderer;

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	renderer.clear();

	// Enable Depth Test & Back-Face Culling
	__glCallVoid(glEnable(GL_DEPTH_TEST));
	__glCallVoid(glDepthMask(GL_TRUE));
	__glCallVoid(glEnable(GL_CULL_FACE));
	__glCallVoid(glFrontFace(GL_CCW));
	__glCallVoid(glCullFace(GL_BACK));

	// Line width for GL_LINES
	__glCallVoid(glLineWidth(1.0f));

	// Also initializes the basic shader
	Shape::init_static_members();

	// Texture Slot 0 - Tree Surface
	Texture* tree_surface_texture_obj = new Texture("../../Data/textures/tree_surface_4k.png");
	tree_surface_texture_obj->bind(0);

	// Texture Slot 1 - Leafs
	Texture* leaf_texture_obj = new Texture("../../Data/textures/leaf.png");
	leaf_texture_obj->bind(1);
	tree_surface_texture_obj->unbind();
	tree_surface_texture_obj->unbind();

	// View matrix - camera
	PerspectiveCamera::init({ 0.0f, 0.0f, height / 2.0f }, { 0.0f, 1.0f, 0.0f }, 60.0f, width, height);
	const Angel::mat4& view_matrix = PerspectiveCamera::view_matrix();
	const Angel::mat4& proj_matrix = PerspectiveCamera::projection_matrix();

	// Draw List
	DrawList list(proj_matrix, view_matrix);
	// Platform surface
	Angel::vec3* platform_surface_pos, * platform_surface_rot, * platform_surface_scale;
	platform_surface_pos = new Angel::vec3(0.0f, -300.0f, 0.0f);
	platform_surface_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
	platform_surface_scale = new Angel::vec3((float)width, 20.0f, (float)height);
	ShapeModel* platform_surface = new ShapeModel(ShapeModel::StaticShape::COL_CUBE,
		platform_surface_pos, platform_surface_rot, platform_surface_scale);
	list.add_shape(platform_surface);

	// Articulated Tree Model
	auto hierarchical_model = new ArticulatedModel(Angel::vec3(0.0f, -280.0f, 0.0f), 
		tree_surface_texture_obj, 0,
		proj_matrix, view_matrix);

	// Selection System
	SelectionSystem3D* selection_system = new SelectionSystem3D(&list, hierarchical_model, width, height);
	int hovered_shape_index = -1;
	int cur_selected_index = -1;

	bool is_dragging = false;
	ImVec2 editor_pane_size(width / 6.0f, (float)height);
	ImGuiColorEditFlags color_edit_flags = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;		
	ImGuiWindowFlags flags_editor_pane =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoResize;
	
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

		glfwPollEvents();

		// Update the viewport
		int old_width = width, old_height = height;
		glfwGetWindowSize(window, &width, &height);

		// Update the window projection
		if (old_width != width || old_height != height)
		{
			PerspectiveCamera::on_viewport_resize(width, height);
			editor_pane_size = { width / 6.0f, (float)height };
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
							// deselect previous
							ArticulatedModelNode* node = hierarchical_model->get_node(cur_selected_index);
							if (node != nullptr)
							{
								node->set_selected(false);
							}
							cur_selected_index = hovered_shape_index;
						}
						else
						{
							std::cout << "THIS IS A DRAG" << std::endl;
							if (cur_selected_index == -1)
							{
								cur_selected_index = hovered_shape_index;
							}
							// Do sth. with drag vector here
						}
						if (cur_selected_index >= hierarchical_model->min_entity_id()
							&& cur_selected_index <= ArticulatedModel::max_entity_id())
						{
							ArticulatedModelNode* node = hierarchical_model->get_node(cur_selected_index);
							if (node != nullptr)
							{
								node->set_selected(true);
							}
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
					if (ImGui::BeginTabItem("Selection"))
					{
						if (cur_selected_index > 0 && cur_selected_index < list.shape_models().size())
						{
							ShapeModel* cur_selected = list.shape_models().at(cur_selected_index);
							ImGui::Text("Selected Entity Type : %s", magic_enum::enum_name<ShapeModel::StaticShape>(cur_selected->shape_def()).data());

							ImGui::NewLine();

							ImGui::SliderFloat("X Position", &cur_selected->position().x, -2000.0f, 2000.0f, "%.1f", 1.0f);
							ImGui::SliderFloat("Z Position", &cur_selected->position().z, -2000.0f, 2000.0f, "%.1f", 1.0f);
							ImGui::SliderFloat("X Rotation", &cur_selected->rotation().x, -180.0f, 180.0f, "%.3f", 1.0f);
							ImGui::SliderFloat("Y Rotation", &cur_selected->rotation().y, -180.0f, 180.0f, "%.3f", 1.0f);
							ImGui::SliderFloat("Z Rotation", &cur_selected->rotation().z, -180.0f, 180.0f, "%.3f", 1.0f);
						}
						else if (cur_selected_index == 0)
						{
							ImGui::Text("Platform surface was selected...");
						}
						else if (cur_selected_index >= hierarchical_model->min_entity_id()
							&& cur_selected_index <= ArticulatedModel::max_entity_id())
						{
							ArticulatedModelNode* node = hierarchical_model->get_node(cur_selected_index);
							if (node == hierarchical_model->torso())
							{
								ImGui::Text("Trunk position: ");
								ImGui::SliderFloat("X Position", &hierarchical_model->position().x, -2000.0f, 2000.0f, "%.1f", 1.0f);
								ImGui::SliderFloat("Z Position", &hierarchical_model->position().z, -2000.0f, 2000.0f, "%.1f", 1.0f);
								ImGui::NewLine();
								ImGui::Text("Trunk rotation: ");
							}
							else
							{
								ImGui::Text("Branch rotation: ");
							}
							ImGui::SliderFloat("X Rotation", &node->rotation_vec().x, -180.0f, 180.0f, "%.3f", 1.0f);
							ImGui::SliderFloat("Y Rotation", &node->rotation_vec().y, -180.0f, 180.0f, "%.3f", 1.0f);
							ImGui::SliderFloat("Z Rotation", &node->rotation_vec().z, -180.0f, 180.0f, "%.3f", 1.0f);
						}
						else
						{
							ImGui::Text("No selection at the moment...");
						}
						

						ImGui::NewLine();
						ImGui::Text("Hovered Entity Index : %d", hovered_shape_index);
						ImGui::Text("Selected Entity Index : %d", cur_selected_index);


						ImGui::NewLine();
						ImGui::Text("Camera-Pos.x: %f", PerspectiveCamera::position().x);
						ImGui::Text("Camera-Pos.y: %f", PerspectiveCamera::position().y);
						ImGui::Text("Camera-Pos.z: %f", PerspectiveCamera::position().z);
						ImGui::Text("Pitch: %f", PerspectiveCamera::pitch());
						ImGui::Text("Yaw: %f", PerspectiveCamera::yaw());
						ImGui::Text("Roll: %f", PerspectiveCamera::roll());

						ImGui::NewLine();
						ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
							1000.0f / ImGui::GetIO().Framerate,
							ImGui::GetIO().Framerate);

					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::End();

		}
		ImGui::EndFrame();

		// Update the selection system
		if (old_width != width || old_height != height)
		{
			selection_system->on_screen_resize(width, height);
		}
		if (window_input.m_mouse_x != -1.0 && 
			window_input.m_mouse_y != -1.0)
		{
			unsigned int idx = selection_system->on_update(
				static_cast<int>(window_input.m_mouse_x),
				static_cast<int>(window_input.m_mouse_y)
			);
			hovered_shape_index = idx - 1;
			if (idx >= hierarchical_model->min_entity_id()
				&& idx <= ArticulatedModel::max_entity_id())
			{
				hovered_shape_index = idx;
			}
			else if (idx > list.shape_models().size())
			{
				printf("\x1B[31mError : Hovered model index was %d\033[0m\n", idx);
				hovered_shape_index = - 1;
			}
		}

		// Clear background
		Renderer::set_viewport(window);
		Renderer::clear();

		// Draw the draw list
		list.draw_all();

		// Draw the model
		hierarchical_model->draw_model();

		// Always draw ImGui on top of the app
		render_imgui();

		glfwSwapBuffers(window);
	}

	// Clear the draw list & delete the VB/IB/VA objects for polygons
	list.shutdown();

	// Cleanup
	Shape::destroy_static_members_allocated_on_the_heap();
	delete selection_system; 
	delete tree_surface_texture_obj;
	delete leaf_texture_obj;
	delete hierarchical_model;

	// Shutdown ImGui & GLFW
	shutdown_imgui();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
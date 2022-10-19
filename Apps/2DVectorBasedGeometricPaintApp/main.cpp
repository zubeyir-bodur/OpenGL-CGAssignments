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
#include "Camera.h"

#include <nothings-stb/stb_image.h>
#include <dearimgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
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
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "CS 465 - Assignment 1 - 2D Vector-based Geometric Paint Application", nullptr, nullptr);
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(!window || !mode)
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

	float init_shape_length = width / 8.0f;

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA));

	// Specify the colors
	float color_sheet[4]	= { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_current[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float color_draw[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float tmp_degree = 0.0f;
	ImVec4 clear_color = ImVec4(0.3984375f, 0.3984375f, 0.3984375f, 1.0f);

	Shape::init_static_members(width);

	Renderer renderer;

	glm::vec3 sheet_pos(0, height/7.0f, 0.0f);

	// View matrix - camera
	constexpr float global_z_pos_2d = 0.0f;
	Camera::init(glm::vec3(0.0f, 0.0f, global_z_pos_2d), 100.0f);
	auto view_matrix = Camera::view_matrix();

	// Orthographic projection is used
	glm::mat4 projection_matrix = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

	// Mouse location
	auto cursor_model_coords = Camera::map_from_global(0, 0);

	// Sheet initializations
	glm::mat4 model_sheet_matrix = glm::translate(glm::mat4(1.0f), sheet_pos)
		* glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, (6.0f/7.0f)*(8.0f*height / width), 1.0f));
	glm::mat4 MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;

	DrawList list(&renderer, projection_matrix, view_matrix);

	enum class RadioButtons
	{
		Select = 0,
		DrawRect = 1,
		DrawEqTri = 2,
		DrawPoly = 3,
		Delete = 4
	};
	int radio_button_cur = (int)RadioButtons::Select;
	const float& imgui_zoom_ratio = Camera::get_zoom_ratio();
	ImGuiColorEditFlags f = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		Input::ButtonState s = window_input.m_lmb_state;
		window_input.m_scroll_y = 0.0;
		glfwPollEvents();

		// Update the projection matrix
		glfwGetWindowSize(window, &width, &height);
		
		sheet_pos = glm::vec3(0, height / 7.0f, 0.0f);
		model_sheet_matrix = glm::translate(glm::mat4(1.0f), sheet_pos)
			* glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, (6.0f / 7.0f) * (8.0f * height / width), 1.0f));
		MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;
		projection_matrix = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

		// Update cursor
		cursor_model_coords = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);

		if (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard)
		{
			// Camera movement - simple
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				Camera::move_vertical(-20.0f);
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				Camera::move_horizontal(-20.0f);
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				Camera::move_vertical(20.0f);
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				Camera::move_horizontal(20.0f);
			}

			// Camera zooming with mouse wheel
			Camera::zoom(window_input.m_scroll_y, window_input.m_mouse_x, window_input.m_mouse_y);

			// TODO
			if (radio_button_cur == (int)RadioButtons::Select)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}
			if (radio_button_cur == (int)RadioButtons::Delete)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}
			if (radio_button_cur == (int)RadioButtons::DrawRect)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}
			if (radio_button_cur == (int)RadioButtons::DrawEqTri)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}
			if (radio_button_cur == (int)RadioButtons::DrawPoly)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}
		}		

		// ImGui Components 
		new_imgui_frame();
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_::ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(width, height/7.0f), ImGuiCond_::ImGuiCond_Always);
		ImGuiWindowFlags flags_editor =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoResize;
		ImGui::Begin("Editor Pane", nullptr, flags_editor);
		{
			if (ImGui::BeginTabBar("##tabs"))
			{
				if (ImGui::BeginTabItem("Home"))
				{
					ImGui::RadioButton("Selection Mode", &radio_button_cur, (int)RadioButtons::Select);
					ImGui::SameLine();
					ImGui::RadioButton("Deletion Mode", &radio_button_cur, (int)RadioButtons::Delete);
					ImGui::SameLine();
					ImGui::RadioButton("Draw Rectangle", &radio_button_cur, (int)RadioButtons::DrawRect);
					ImGui::SameLine();
					ImGui::RadioButton("Draw Eq. Triangle", &radio_button_cur, (int)RadioButtons::DrawEqTri);
					ImGui::SameLine();
					ImGui::RadioButton("Draw Convex Poly", &radio_button_cur, (int)RadioButtons::DrawPoly);
					ImGui::SameLine();
					if (ImGui::Button("Undo"))
					{
						// TO DO Undo operation
					}
					ImGui::SameLine();
					if (ImGui::Button("Redo"))
					{
						// TO DO Redo operation
					}
					ImGui::SameLine();
					if (ImGui::Button("Save"))
					{
						// TO DO Undo operation
					}
					ImGui::SameLine();
					if (ImGui::Button("Load"))
					{
						// TO DO Redo operation
					}
					if (radio_button_cur == (int)RadioButtons::DrawEqTri
						|| radio_button_cur == (int)RadioButtons::DrawRect
						|| radio_button_cur == (int)RadioButtons::DrawPoly)
					{
						ImGui::ColorEdit4("Drawing Color", &color_draw[0], f);
					}
					ImGui::Text("Cursor Model Coordinates: %f, %f", cursor_model_coords.x, cursor_model_coords.y);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Selection"))
				{
					ImGui::Text("Currently, there is no shape selected");
					ImGui::SameLine();
					ImGui::Text("Multiple shapes were selected");
					ImGui::ColorEdit4("Shape Color", &color_current[0], f);
					ImGui::SameLine();
					ImGui::SliderFloat("Rotation Degree", &tmp_degree, 0.0f, 360, "%.3f", 1.0f);
					ImGui::Text("Size of Selected Shape: %f, %f", 0,0);
					ImGui::Text("Position of the Center of the Shape: %f, %f", 0, 0);
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		ImGui::ShowDemoWindow(nullptr);

		// Clear background
		renderer.set_viewport(window);
		renderer.clear((float*)&clear_color);

		// Get cursor model coordinates
		ImGui::EndFrame();

		// Shader for sheet
		Shape::shader()->bind();
		Shape::shader()->set_uniform_4f("u_color",
			color_sheet[0],
			color_sheet[1],
			color_sheet[2],
			color_sheet[3]);
		view_matrix = Camera::view_matrix();
		MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;
		Shape::shader()->set_uniform_mat4f("u_MVP", MVP_mat_sheet);

		// Draw the sheet
		renderer.draw_triangles(Shape::rectangle()->vertex_array(), Shape::rectangle()->index_buffer(), Shape::shader());

		// Draw the draw list
		list.draw_all();

		// Always draw ImGui on top of the app
		render_imgui();

		glfwSwapBuffers(window);

		// Next state logic for LMB
		if (s == Input::ButtonState::BeingPressed
			&& window_input.m_lmb_state == Input::ButtonState::Released)
		{
			// Consume the released state
			window_input.m_mouse_release_y = -1.0f;
			window_input.m_mouse_release_x = -1.0f;
			window_input.m_mouse_press_y = -1.0f;
			window_input.m_mouse_press_x = -1.0f;
			window_input.m_lmb_state = Input::ButtonState::Idle;
			std::cout << "LMB is now Idle" << std::endl;
		}
		if (s == Input::ButtonState::Idle
			&& window_input.m_lmb_state == Input::ButtonState::JustPressed)
		{
			// Consume the released state
			window_input.m_lmb_state = Input::ButtonState::BeingPressed;
			std::cout << "LMB is now BeingPressed" << std::endl;
		}
	}

	Shape::destroy_static_members_allocated_on_the_heap();

	// Cleanup
	shutdown_imgui();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
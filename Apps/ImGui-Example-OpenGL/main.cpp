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
		GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Dear ImGui Example For OpenGL and GLFW for testing the Rendering Engine", nullptr, nullptr);
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
		__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA));

		// Specify the color of the triangle
		float color_sheet[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 color_a = { 0.6f, 0.9f, 0.0f, 1.0f };
		glm::vec4 color_b = { 0.9f, 0.6f, 0.0f, 1.0f };
		glm::vec4 color_c = { 1.0f, 0.0f, 0.0f, 1.0f };

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

		glm::vec3 sheet_pos(0, 0.0f, 0.0f);

		// Initialize shapes
		// positions - respect to their initial 0th vertex positions
		glm::vec3 model_a_pos(0, 0.0f, 0.0f);
		glm::vec3 model_b_pos(width / 2.0f - init_shape_length / 2, height / 2.0f - init_shape_length / 2, 0.0f);
		glm::vec3 model_c_pos(width / 4.0f - init_shape_length / 2, height / 4.0f - init_shape_length / 2, 0.0f);

		// rotation - in radians (x, y, z axises respectively)
		glm::vec3 model_a_rot(0.0f, 0, 0.0f);
		glm::vec3 model_b_rot(0.0f, 0.0f, 0.0f);
		glm::vec3 model_c_rot(0.0f, 0.0f, 0.0f);
		// scale
		glm::vec3 model_a_scale(1.0f, 1.0f, 1.0f);
		glm::vec3 model_b_scale(1.0f, 1.0f, 1.0f);
		glm::vec3 model_c_scale(1.0f, 1.0f, 1.0f);

		ShapeModel model_a(ShapeModel::StaticShape::RECTANGLE,
			&model_a_pos,
			&model_a_rot,
			&model_a_scale,
			&color_a
		);
		ShapeModel model_b(ShapeModel::StaticShape::RECTANGLE,
			&model_b_pos,
			&model_b_rot,
			&model_b_scale,
			&color_b
		);
		ShapeModel model_c(ShapeModel::StaticShape::EQUILATERAL_TRIANGLE,
			&model_c_pos,
			&model_c_rot,
			&model_c_scale,
			&color_c
		);

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
			* glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, 8.0f * mode->height / (float)mode->width, 1.0f));
		glm::mat4 MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;

		DrawList list(&renderer, projection_matrix, view_matrix);
		list.add_shape(&model_a);
		list.add_shape(&model_b);
		list.add_shape(&model_c);
		bool should_select = true;
		bool should_draw_rect = false;
		bool should_draw_eq_tri = false;
		bool should_draw_convex_poly = false;
		bool should_delete_shape = false;
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
			projection_matrix = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

			// Update cursor
			cursor_model_coords = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);

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

			if (should_draw_rect)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}

			if (should_draw_eq_tri)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}

			if (should_draw_convex_poly)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}

			if (should_delete_shape)
			{
				if (window_input.m_lmb_state == Input::ButtonState::JustPressed)
				{

				}
				if (window_input.m_lmb_state == Input::ButtonState::Released)
				{

				}
			}

			// update center positions of models for logging
			glm::vec2 size_a = model_a.shape_size();
			glm::vec2 center_a = model_a.center_position();
			glm::vec2 size_b = model_b.shape_size();
			glm::vec2 center_b = model_b.center_position();
			glm::vec2 size_c = model_c.shape_size();
			glm::vec2 center_c = model_c.center_position();

			// ImGui Components 
			new_imgui_frame();

			ImGui::Begin("Hello, world!");

			ImGui::Text("This is some useful text.");

			ImGui::SliderFloat("Model A-Xpos", &model_a_pos.x, 0.0f, (float)mode->width - size_a.x, "%.1f", 1.0f);
			ImGui::SliderFloat("Model A-Ypos", &model_a_pos.y, 0.0f, (float)mode->height - size_a.y, "%.1f", 1.0f);
			ImGui::SliderFloat("Model A-zrot", &model_a_rot.z, 0.0f, 360, "%.3f", 1.0f);
			ImGui::Text("Size of Model A: %f, %f", size_a.x, size_a.y);
			ImGui::Text("Position of the Center of Model A: %f, %f", center_a.x, center_a.y);
			ImGui::NewLine();

			ImGui::SliderFloat("Model B-XPos", &model_b_pos.x, 0.0f, (float)mode->width - size_b.x, "%.1f", 1.0f);
			ImGui::SliderFloat("Model B-YPos", &model_b_pos.y, 0.0f, (float)mode->height - size_b.y, "%.1f", 1.0f);
			ImGui::SliderFloat("Model B-zrot", &model_b_rot.z, 0.0f, 360, "%.3f", 1.0f);
			ImGui::Text("Size of Model B: %f, %f", size_b.x, size_b.y);
			ImGui::Text("Position of the Center of Model B: %f, %f", center_b.x, center_b.y);
			ImGui::NewLine();

			ImGui::SliderFloat("Model C-XPos", &model_c_pos.x, 0.0f, (float)mode->width - size_c.x, "%.1f", 1.0f);
			ImGui::SliderFloat("Model C-YPos", &model_c_pos.y, 0.0f, (float)mode->height - size_c.y, "%.1f", 1.0f);
			ImGui::SliderFloat("Model C-zrot", &model_c_rot.z, 0.0f, 360, "%.3f", 1.0f);
			ImGui::Text("Size of Model C: %f, %f", size_c.x, size_c.y);
			ImGui::Text("Position of the Center of Model C: %f, %f", center_c.x, center_c.y);
			ImGui::NewLine();

			if (!has_texture)
			{
				ImGui::ColorEdit4("Model A Color", &color_a.x, f);
				ImGui::SameLine();
				ImGui::ColorEdit4("Model B Color", &color_b.x, f);
				ImGui::SameLine();
				ImGui::ColorEdit4("Model C Color", &color_c.x, f);
				ImGui::NewLine();
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
				1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

			// Clear background
			renderer.set_viewport(window);
			renderer.clear((float*)&clear_color);

			// Get cursor model coordinates
			ImGui::Text("Cursor Model Coordinates: %f, %f", cursor_model_coords.x, cursor_model_coords.y);
			ImGui::End();
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

		// Cleanup
		Shape::destroy_static_members_allocated_on_the_heap();
		shutdown_imgui();

		glfwDestroyWindow(window);
		glfwTerminate();

		return 0;
	}
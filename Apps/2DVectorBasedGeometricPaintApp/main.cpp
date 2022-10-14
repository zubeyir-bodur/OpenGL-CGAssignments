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

#include <nothings-stb/stb_image.h>
#include <dearimgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glew.h>
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
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "CS 465 - Assignment 1 - 2D Vector-based Geometric Paint Application", NULL, NULL);
	if(!window || !mode)
	{
		glfwTerminate();
		return -1;
	}
	glfwSetWindowAspectRatio(window, 16, 9); // force 16:9 aspect ratio
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

	// Setup Dear ImGui context
	auto cfg = init_imgui(window);
	SetupImGuiStyle();
	// ImGui state
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

	// The triangles
	constexpr unsigned int num_vertices = 4;
	constexpr unsigned int num_coordinates = 3;
	constexpr unsigned int num_indices = 6;
	float positions[num_vertices * num_coordinates] = {
		0.0f,                 0.0f                , 0.0f,	// 0
		(mode->width) / 8.0f, 0.0f                , 0.0f,	// 1
		(mode->width) / 8.0f, (mode->width) / 8.0f, 0.0f,	// 2
		0.0f,                 (mode->width) / 8.0f, 0.0f	// 3
	};


	float sheet_positions[num_vertices * num_coordinates] = {
		0.0f,                      0.0f               , 0.0f,	// 0
		(float)mode->width,        0.0f               , 0.0f,	// 1
		(float)mode->width,        (float)mode->height, 0.0f,	// 2
		0,                         (float)mode->height, 0.0f	// 3
	};

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA));


	// Create the vertex buffer for sheet
	auto* sheet_va = new VertexArray;
	auto* sheet_vb = new VertexBuffer(sheet_positions, num_vertices * num_coordinates * sizeof(float));
	auto* sheet_layout = new VertexBufferLayout();
	sheet_layout->push_back_elements<float>(num_coordinates);
	sheet_va->add_buffer(*sheet_vb, *sheet_layout);

	// Create the vertex buffer for a square
	auto* square_va = new VertexArray;
	auto* square_vb = new VertexBuffer(positions, num_vertices * num_coordinates * sizeof(float));
	auto* square_layout = new VertexBufferLayout();
	square_layout->push_back_elements<float>(num_coordinates);
	square_va->add_buffer(*square_vb, *square_layout);

	// Index buffer for a 2D quad
	unsigned int quad_indicies_2d[num_indices] = {
		0, 1, 2,
		2, 3, 0
	};
	auto* quad_ib = new IndexBuffer(quad_indicies_2d, num_indices);

	// Compile & bind shaders
	auto* shader_raw = new Shader("../../Common/shaders/triangle.glsl");

	// Specify the color of the triangle
	float sheet_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float triangle_color_a[4] = { 0.6f, 0.9f, 0.0f, 1.0f };
	float triangle_color_b[4] = { 0.9f, 0.6f, 0.0f, 1.0f };

	// Unbind all, as no longer binding needed
	sheet_va->unbind();
	square_va->unbind();
	square_vb->unbind();
	sheet_vb->unbind();
	quad_ib->unbind();
	shader_raw->unbind();

	Renderer renderer;

	glm::vec3 sheet_pos(0, 0.0f, 0.0f);

	// Initialize two equivalent shapes
	// position
	glm::vec3 model_a_pos(0, 0.0f, 0.0f);
	glm::vec3 model_b_pos(mode->width / 2.0f - mode->width / 16.0f, mode->height / 2.0f - mode->width / 16.0f, 0.0f);
	std::vector<glm::vec2> std_positions;
	for (int i = 0; i < num_vertices; i++)
	{
 		std_positions.emplace_back(
 				*(positions + i * square_layout->stride()),                  // x coordinate of the vertices
 				*(positions + i * square_layout->stride() + sizeof(float))   // y coordinate of the vertices
		);
	}
	// rotation - in radians (x, y, z axises respectively)
	glm::vec3 model_a_rot(0.0f, 0, 0.0f);
	glm::vec3 model_b_rot(0.0f, 0.0f, 0.0f);
	// scale
	glm::vec3 model_a_scale(1.0f, 1.0f, 1.0f);
	glm::vec3 model_b_scale(1.0f, 1.0f, 1.0f);

	Shape model_a{}, model_b{};
	model_b.m_index_buffer = model_a.m_index_buffer = quad_ib;
	model_b.m_layout = model_a.m_layout = square_layout;
	model_b.m_shader = model_a.m_shader = shader_raw;
	model_b.m_vertex_array = model_a.m_vertex_array = square_va;
	model_b.m_vertex_coordinates = model_a.m_vertex_coordinates = &std_positions;

	model_a.m_color = static_cast<float*>(triangle_color_a);
	model_a.m_position = &model_a_pos;
	model_a.m_rotation = &model_a_rot;
	model_a.m_scale = &model_a_scale;

	model_b.m_color = static_cast<float*>(triangle_color_b);
	model_b.m_position = &model_b_pos;
	model_b.m_rotation = &model_b_rot;
	model_b.m_scale = &model_b_scale;

	// View matrix - camera
	glm::vec3 camera_pos(0.0f);
	glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), -camera_pos);

	// Project into window content coordinate system
	glm::mat4 projection_matrix = glm::ortho(0.0f, (float)mode->width, (float)mode->height, 0.0f, -1.0f, 1.0f);

	DrawList list(&renderer, projection_matrix, view_matrix);
	list.add_shape(&model_a);
	list.add_shape(&model_b);

	ImGuiColorEditFlags f = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// ImGui Components 
		new_imgui_frame();

		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");

		ImGui::SliderFloat("Model A-Xpos", &model_a_pos.x, 0.0f, (float)mode->width  - (mode->width) / 8.0f, "%.1f", 1.0f);
		ImGui::SliderFloat("Model A-Ypos", &model_a_pos.y, 0.0f, (float)mode->height - (mode->width) / 8.0f, "%.1f", 1.0f);
		ImGui::SliderFloat("Model A-xrot", &model_a_rot.x, 0.0f, 3.14f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model A-yrot", &model_a_rot.y, 0.0f, 3.14f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model A-zrot", &model_a_rot.z, 0.0f, 3.14f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model A-Xscale", &model_a_scale.x, 0.0f, 3.0f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model A-Yscale", &model_a_scale.y, 0.0f, 3.0f, "%.3f", 1.0f);
		ImGui::NewLine();

		ImGui::SliderFloat("Model B-XPos", &model_b_pos.x, 0.0f, (float)mode->width - (mode->width) / 8.0f, "%.1f", 1.0f);
		ImGui::SliderFloat("Model B-YPos", &model_b_pos.y, 0.0f, (float)mode->height - (mode->width) / 8.0f, "%.1f", 1.0f);
		ImGui::SliderFloat("Model B-xrot", &model_b_rot.x, 0.0f, 3.14f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model B-yrot", &model_b_rot.y, 0.0f, 3.14f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model B-zrot", &model_b_rot.z, 0.0f, 3.14f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model B-Xscale", &model_b_scale.x, 0.0f, 3.0f, "%.3f", 1.0f);
		ImGui::SliderFloat("Model B-Yscale", &model_b_scale.y, 0.0f, 3.0f, "%.3f", 1.0f);
		ImGui::NewLine();

		ImGui::SliderFloat2("Camera Coordinates", &camera_pos.x, (float)mode->width / -2.0f, (float)mode->width / 2.0f, "%.3f", 1.0f);
		ImGui::NewLine();

		ImGui::ColorEdit3("Clear Color", (float*)&clear_color, f);
		ImGui::SameLine();
		ImGui::ColorEdit4("Model A Color", triangle_color_a, f);
		ImGui::SameLine();
		ImGui::ColorEdit4("Model B Color", triangle_color_b, f);
		ImGui::NewLine();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
			1000.0f / ImGui::GetIO().Framerate,
			ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::EndFrame();

		// Clear background
		renderer.set_viewport(window);
		renderer.clear((float*)&clear_color);

		// Update camera position
		view_matrix = glm::translate(glm::mat4(1.0f), -camera_pos);

		// Model matrix for the sheet
		glm::mat4 model_sheet_matrix = glm::translate(glm::mat4(1.0f), sheet_pos);
		glm::mat4 MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;
		shader_raw->bind();
		shader_raw->set_uniform_4f("u_color",
			sheet_color[0],
			sheet_color[1],
			sheet_color[2],
			sheet_color[3]);
		shader_raw->set_uniform_mat4f("u_MVP", MVP_mat_sheet);

		// Draw the sheet
		renderer.draw(sheet_va, quad_ib, shader_raw);

		// Draw the draw list
		list.draw_all();

		// Always draw ImGui on top of the app
		render_imgui();

		glfwSwapBuffers(window);
	}

	// delete for each new
	delete square_vb;
	delete quad_ib;
	delete sheet_va;
	delete square_layout;
	delete shader_raw;

	// Cleanup
	shutdown_imgui(cfg);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
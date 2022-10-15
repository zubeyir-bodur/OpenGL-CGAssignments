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

#include <nothings-stb/stb_image.h>
#include <dearimgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
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
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "CS 465 - Assignment 1 - 2D Vector-based Geometric Paint Application", nullptr, nullptr);
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
	init_imgui(window);
	SetupImGuiStyle();
	// ImGui state
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);


	float init_shape_length = (mode->width) / 8.0f;

	// The triangles
	constexpr float global_z_pos_2d = 0.5f;
	constexpr unsigned int rect_num_vertices = 4;
	constexpr unsigned int num_coordinates = 3;
	constexpr unsigned int num_indices = 6;
	float rectangle_positions[rect_num_vertices * num_coordinates] = {
		0.0f,					0.0f                , global_z_pos_2d,	// 0
		init_shape_length,		0.0f                , global_z_pos_2d,	// 1
		init_shape_length,		init_shape_length	, global_z_pos_2d,	// 2
		0.0f,					init_shape_length	, global_z_pos_2d	// 3
	};

	constexpr unsigned int tri_num_vertices = 3;
	float equilateral_triangle_positions[tri_num_vertices * num_coordinates] = {
		init_shape_length/2,	0.0f							, global_z_pos_2d,	// 0
		0.0f,					sqrtf(3)* init_shape_length / 2  , global_z_pos_2d,	// 1
		init_shape_length,		sqrtf(3)* init_shape_length / 2	, global_z_pos_2d,	// 2
	};

	// Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA));

	// Single color, texture-less, 3D vertex buffer layout
	auto* basic_layout = new VertexBufferLayout();
	basic_layout->push_back_elements<float>(num_coordinates);

	// Create the vertex buffer for a rectangle
	auto* rect_va = new VertexArray;
	auto* rect_vb = new VertexBuffer(rectangle_positions, rect_num_vertices * num_coordinates * sizeof(float));
	rect_va->add_buffer(*rect_vb, *basic_layout);

	// Vertex Buffer for an equilateral triangle
	auto* eq_tri_va = new VertexArray;
	auto* eq_tri_vb = new VertexBuffer(equilateral_triangle_positions, tri_num_vertices * num_coordinates * sizeof(float));
	eq_tri_va->add_buffer(*eq_tri_vb, *basic_layout);

	// Index buffer for a 2D quad
	unsigned int rect_indices[num_indices] = {
		0, 1, 2,
		2, 3, 0
	};
	auto* rect_ib = new IndexBuffer(rect_indices, num_indices);
	auto* eq_tri_ib = new IndexBuffer(rect_indices, num_indices/2);

	// Compile & bind shaders
	auto* triangle_shader = new Shader("../../Common/shaders/triangle.glsl");

	// Specify the color of the triangle
	float color_sheet[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_a[4] = { 0.6f, 0.9f, 0.0f, 1.0f };
	float color_b[4] = { 0.9f, 0.6f, 0.0f, 1.0f };
	float color_c[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

	// Unbind all, as no longer binding needed
	rect_va->unbind();
	rect_vb->unbind();
	rect_ib->unbind();
	eq_tri_va->unbind();
	eq_tri_vb->unbind();
	eq_tri_ib->unbind();
	triangle_shader->unbind();

	Renderer renderer;

	glm::vec3 sheet_pos(0, 0.0f, 0.0f);

	// Initialize shapes
	// positions - respect to their initial 0th vertex positions
	glm::vec3 model_a_pos(0, 0.0f, 0.0f);
	glm::vec3 model_b_pos(mode->width / 2.0f - init_shape_length/2, mode->height / 2.0f - init_shape_length / 2, 0.0f);
	glm::vec3 model_c_pos(mode->width / 4.0f - init_shape_length / 2, mode->height / 4.0f - init_shape_length / 2, 0.0f);

	// copy the initial model positions
	std::vector<glm::vec3> std_rect_positions;
	for (int i = 0; i < rect_num_vertices; i++)
	{
		float* x_ptr = rectangle_positions + i * basic_layout->tot_elements();
		float* y_ptr = rectangle_positions + i * basic_layout->tot_elements() + 1;
		float* z_ptr = rectangle_positions + i * basic_layout->tot_elements() + 2;
		float x = *x_ptr;
		float y = *y_ptr;
		float z = *z_ptr;
 		std_rect_positions.emplace_back(glm::vec3(x, y, z));
	}

	std::vector<glm::vec3> std_triangle_positions;
	for (int i = 0; i < tri_num_vertices; i++)
	{
		float* x_ptr = equilateral_triangle_positions + i * basic_layout->tot_elements();
		float* y_ptr = equilateral_triangle_positions + i * basic_layout->tot_elements() + 1;
		float* z_ptr = equilateral_triangle_positions + i * basic_layout->tot_elements() + 2;
		float x = *x_ptr;
		float y = *y_ptr;
		float z = *z_ptr;
		std_triangle_positions.emplace_back(glm::vec3(x, y, z));
	}

	// rotation - in radians (x, y, z axises respectively)
	glm::vec3 model_a_rot(0.0f, 0, 0.0f);
	glm::vec3 model_b_rot(0.0f, 0.0f, 0.0f);
	glm::vec3 model_c_rot(0.0f, 0.0f, 0.0f);
	// scale
	glm::vec3 model_a_scale(1.0f, 1.0f, 1.0f);
	glm::vec3 model_b_scale(1.0f, 1.0f, 1.0f);
	glm::vec3 model_c_scale(1.0f, 1.0f, 1.0f);

	Shape model_a{}, model_b{}, model_c{};
	model_b.m_index_buffer = model_a.m_index_buffer = rect_ib;
	model_c.m_layout = model_b.m_layout = model_a.m_layout = basic_layout;
	model_c.m_shader = model_b.m_shader = model_a.m_shader = triangle_shader;
	model_b.m_vertex_array = model_a.m_vertex_array = rect_va;
	model_b.m_vertex_coordinates = model_a.m_vertex_coordinates = &std_rect_positions;

	model_a.m_color = static_cast<float*>(color_a);
	model_a.m_position = &model_a_pos;
	model_a.m_rotation = &model_a_rot;
	model_a.m_scale = &model_a_scale;

	model_b.m_color = static_cast<float*>(color_b);
	model_b.m_position = &model_b_pos;
	model_b.m_rotation = &model_b_rot;
	model_b.m_scale = &model_b_scale;

	model_c.m_index_buffer = eq_tri_ib;
	model_c.m_vertex_array = eq_tri_va;
	model_c.m_vertex_coordinates = &std_triangle_positions;
	model_c.m_color = static_cast<float*>(color_c);
	model_c.m_position = &model_c_pos;
	model_c.m_rotation = &model_c_rot;
	model_c.m_scale = &model_c_scale;

	constexpr float global_aspect_ratio = 16.0f / 9.0f;
	constexpr float global_fovy = 90.0f;
	auto z_far = (float)mode->width;
	// View matrix - camera - since the app is 2D, it is not affected any of these values unless the camera is ahead of the z=0.5 plane
	glm::vec3 camera_pos(0.0f, 0.0f, mode->width*global_z_pos_2d / (global_aspect_ratio*tan(glm::radians(global_fovy / 2.0f))));
	glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), -camera_pos);

	// Project into window content coordinate system
	
	// old orthogonal projection, does not support zooming and out
	// glm::mat4 projection_matrix = glm::ortho(0.0f, (float)mode->width, (float)mode->height, 0.0f, -1.0f, 1.0f);
	
	// new perspective projection
	// FOV does not really matter as z positions will be the same
	glm::mat4 projection_matrix = glm::perspective(glm::radians(global_fovy), global_aspect_ratio, 0.0f, z_far)
		* glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, // symmetry w.r.t. x-axis is necessary for imitating GLFW window coordinate system
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f)
		* glm::translate(glm::mat4(1.0f), glm::vec3(-mode->width/2.0f, -mode->height/2.0f, 0.0f)); // and also moving the origin to the top left of the screen for GLFW 

	// Sheet initializations
	glm::mat4 model_sheet_matrix = glm::translate(glm::mat4(1.0f), sheet_pos)
		* glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, 8.0f*mode->height / (float)mode->width, 1.0f));
	glm::mat4 MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;

	DrawList list(&renderer, projection_matrix, view_matrix);
	list.add_shape(&model_a);
	list.add_shape(&model_b);
	list.add_shape(&model_c);

	ImGuiColorEditFlags f = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

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

		ImGui::SliderFloat("Model A-Xpos", &model_a_pos.x, 0.0f, (float)mode->width  - size_a.x, "%.1f", 1.0f);
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

		ImGui::SliderFloat2("Camera Coordinates", &camera_pos.x, -(float)mode->width, (float)mode->width, "%.3f", 1.0f);
		ImGui::SliderFloat("Camera Zoom", &camera_pos.z, 0.51f, (float)mode->width, "%.3f", 1.0f);
		ImGui::NewLine();

		ImGui::ColorEdit3("Clear Color", (float*)&clear_color, f);
		ImGui::SameLine();
		ImGui::ColorEdit4("Model A Color", color_a, f);
		ImGui::SameLine();
		ImGui::ColorEdit4("Model B Color", color_b, f);
		ImGui::SameLine();
		ImGui::ColorEdit4("Model C Color", color_c, f);
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

		// Shader for sheet
		triangle_shader->bind();
		triangle_shader->set_uniform_4f("u_color",
			color_sheet[0],
			color_sheet[1],
			color_sheet[2],
			color_sheet[3]);
		MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;
		triangle_shader->set_uniform_mat4f("u_MVP", MVP_mat_sheet);

		// Draw the sheet
		renderer.draw(rect_va, rect_ib, triangle_shader);

		// Draw the draw list
		list.draw_all();

		// Always draw ImGui on top of the app
		render_imgui();

		glfwSwapBuffers(window);
	}

	// delete for each new
	delete rect_va;
	delete rect_vb;
	delete rect_ib;
	delete basic_layout;
	delete eq_tri_va;
	delete eq_tri_vb;
	delete eq_tri_ib;
	delete triangle_shader;

	// Cleanup
	shutdown_imgui();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
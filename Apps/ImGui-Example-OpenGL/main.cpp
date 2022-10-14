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
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Dear ImGui GLFW + OpenGL example", nullptr, nullptr);
    if (!window || !mode)
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
	constexpr unsigned int num_coordinates = 2;
    #define has_texture false
    constexpr unsigned int num_coord_p_vertex = (has_texture ? num_coordinates : 0) + num_coordinates;
	constexpr unsigned int num_indices = 6;
    float positions[num_vertices * num_coord_p_vertex] = { // Vertex + Texture positions
    #if has_texture
		0.0f,                 0.0f,                 0.0f, 0.0f,  // 0 ==> Our triangles will form a square at the middle of the window
        (mode->width) / 8.0f, 0.0f,                 1.0f, 0.0f,  // 1    with side length equal to quarter of the screen width
        (mode->width) / 8.0f, (mode->width) / 8.0f, 1.0f, 1.0f,  // 2    and also we assume that the initial vertex buffer coordinates come from window
        0,                    (mode->width) / 8.0f, 0.0f, 1.0f   // 3
    #else
		0.0f,                 0.0f,                    // 0
		(mode->width) / 8.0f, 0.0f,                    // 1
		(mode->width) / 8.0f, (mode->width) / 8.0f,    // 2
		0,                    (mode->width) / 8.0f     // 3
    #endif
    };
	unsigned int indices[num_indices] = {
		0, 1, 2,
		2, 3, 0
	};

    float sheet_positions[num_vertices * num_coordinates] = {
		0.0f,                      0.0f,                // 0
		(float)mode->width,        0.0f,                // 1
        (float)mode->width,        (float)mode->height, // 2
        0,                         (float)mode->height  // 3
    };

    // Enable blending
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA));

	// Init vertex array object
    auto* vertex_array_obj = new VertexArray;

	// Create the vertex buffer
    auto* vertex_buffer_obj = new VertexBuffer(positions, num_vertices*num_coord_p_vertex *sizeof(float));
    auto* vertex_buffer_layout = new VertexBufferLayout();
	vertex_buffer_layout->push_back_elements<float>(num_coordinates);
	if (has_texture) vertex_buffer_layout->push_back_elements<float>(num_coordinates);
	vertex_array_obj->add_buffer(*vertex_buffer_obj, *vertex_buffer_layout);

	// Create index buffer of the prev vertex buffer
    auto* index_buffer_obj = new IndexBuffer(indices, num_indices);

	// Create vertex buffer for the sheet
    auto* sheet_vertex_array_obj = new VertexArray;
    auto* sheet_vb_obj = new VertexBuffer(sheet_positions, num_vertices * num_coordinates * sizeof(float));
    auto* sheet_layout = new VertexBufferLayout();
    sheet_layout->push_back_elements<float>(num_coordinates);
    sheet_vertex_array_obj->add_buffer(*sheet_vb_obj, *sheet_layout);

	// Create index buffer of the sheet
    auto* sheet_idx_buffer = new IndexBuffer(indices, num_indices);

	// Compile & bind shaders
    auto* shader_raw = new Shader("../../Common/shaders/triangle.glsl");

    
	// Texture
	Texture* texture_obj;
    auto* shader_texture = new Shader("../../Common/shaders/textured_triangle.glsl");;
    if (has_texture)
	{
		texture_obj = new Texture("../../Data/textures/eye.png");
		texture_obj->bind();
        shader_texture->set_uniform_1i("u_texture", 0);
    }
    
	// Specify the color of the triangle
    float sheet_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float triangle_color_a[4] = { 0.6f, 0.9f, 0.0f, 1.0f };
	float triangle_color_b[4] = { 0.9f, 0.6f, 0.0f, 1.0f };
    
    // Unbind all, as no longer binding needed
	vertex_array_obj->unbind();
    vertex_buffer_obj->unbind();
	index_buffer_obj->unbind();
	sheet_vb_obj->unbind();
    sheet_idx_buffer->unbind();
    sheet_vertex_array_obj->unbind();
    if (has_texture) shader_texture->unbind();
	shader_raw->unbind();

    Renderer renderer;

	glm::vec3 sheet_pos(0, 0.0f, 0.0f);

    // Initialize two equivalent shapes
    // pos
	glm::vec3 model_a_pos(0, 0.0f, 0.0f);
	glm::vec3 model_b_pos(mode->width / 2.0f - mode->width / 16.0f, mode->height / 2.0f - mode->width / 16.0f, 0.0f);
	// rotation - in radians (x, y, z axises respectively)
	glm::vec3 model_a_rot(0.0f, 0, 0.0f);
	glm::vec3 model_b_rot(0.0f, 0.0f, 0.0f);
	// scale
	glm::vec3 model_a_scale(1.0f, 1.0f, 1.0f);
	glm::vec3 model_b_scale(1.0f, 1.0f, 1.0f);
	std::vector<glm::vec2> std_positions;
	for (int i = 0; i < num_vertices; i++)
	{
		std_positions.emplace_back(
                *(positions + i*vertex_buffer_layout->stride()),                    // x coordinate of the vertices
                *(positions + i * vertex_buffer_layout->stride() + sizeof(float))   // y coordinate of the vertices
);
	}

    Shape model_a{}, model_b{};
    model_b.m_index_buffer = model_a.m_index_buffer = index_buffer_obj;
    model_b.m_layout = model_a.m_layout = vertex_buffer_layout;
    model_b.m_shader = model_a.m_shader = has_texture ? shader_texture: shader_raw;
	model_b.m_vertex_array = model_a.m_vertex_array = vertex_array_obj;
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

		ImGui::SliderFloat2("Model A Coordinates", &model_a_pos.x, -500.0f, (float)mode->width, "%.1f", 1.0f);
		ImGui::SliderFloat2("Model B Coordinates", &model_b_pos.x, -500.0f, (float)mode->width, "%.1f", 1.0f);
        ImGui::SliderFloat2("Camera Coordinates", &camera_pos.x, (float)mode->width / -2.0f, (float)mode->width / 2.0f, "%.3f", 1.0f);

		ImGui::NewLine();
		ImGui::ColorEdit3("Clear Color", (float*)&clear_color, f);

		if (!has_texture)
		{
            ImGui::SameLine();
			ImGui::ColorEdit4("Model A Color", triangle_color_a, f);
			ImGui::SameLine();
			ImGui::ColorEdit4("Model B Color", triangle_color_b, f);
            ImGui::NewLine();
		}

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
		renderer.draw(sheet_vertex_array_obj, sheet_idx_buffer, shader_raw);

        // Draw the draw list
		list.draw_all();

		// Always draw ImGui on top of the app
        render_imgui();

        glfwSwapBuffers(window);
    }

    // delete for each new
    delete vertex_buffer_obj;
    delete index_buffer_obj;
    delete vertex_array_obj;
    delete vertex_buffer_layout;
    delete shader_raw;
    if (has_texture) delete shader_texture;
    if (has_texture) delete texture_obj;

    // Cleanup
    shutdown_imgui(cfg);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
#include "glew.h"
#include "glfw3.h"

#include "ImGuiManager.h"
#include "ErrorManager.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "nothings-stb/stb_image.h"
#include "dearimgui/imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
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
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Dear ImGui GLFW + OpenGL example", NULL, NULL);
    if (window == NULL || mode == NULL)
	{
		glfwTerminate();
		return -1;
    }
	glfwSetWindowAspectRatio(window, 16, 9); // force 16:9 aspect ratio
	glfwMakeContextCurrent(window);
	glfwMaximizeWindow(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    auto cfg = init_imgui(window);

    // Init GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Could not init GLEW..." << std::endl;

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
        0.0f,                       0.0f,         // 4
        (float)mode->width,        0.0f,         // 5
        (float)mode->width,        (float)mode->height, // 6
        0,                          (float)mode->height  // 7
    };

    // Enable blending
    if (has_texture)
    {
		__glCallVoid(glEnable(GL_BLEND));
		__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA));
    }

	// Init vertex array object
    VertexArray* vertex_array_obj = new VertexArray;

	// Create the vertex buffer
    VertexBuffer* vertex_buffer_obj = new VertexBuffer(positions, num_vertices*num_coord_p_vertex *sizeof(float));
    VertexBufferLayout* vertex_buffer_layout = new VertexBufferLayout();
	vertex_buffer_layout->push_back_elements<float>(num_coordinates);
	if (has_texture) vertex_buffer_layout->push_back_elements<float>(num_coordinates);
	vertex_array_obj->add_buffer(*vertex_buffer_obj, *vertex_buffer_layout);

	// Create index buffer of the prev vertex buffer
	IndexBuffer* index_buffer_obj = new IndexBuffer(indices, num_indices);

	// Create vertex buffer for the sheet
	VertexArray* sheet_vertex_array_obj = new VertexArray;
    VertexBuffer* sheet_vb_obj = new VertexBuffer(sheet_positions, num_vertices * num_coordinates * sizeof(float));
    VertexBufferLayout* sheet_layout = new VertexBufferLayout();
    sheet_layout->push_back_elements<float>(num_coordinates);
    sheet_vertex_array_obj->add_buffer(*sheet_vb_obj, *sheet_layout);

	// Create index buffer of the sheet
	IndexBuffer* sheet_idx_buffer = new IndexBuffer(indices, num_indices);

	// Compile & bind shaders
	Shader* shader_raw = new Shader("../../Common/shaders/triangle.glsl");

    // Project into window content coordinate system
    glm::mat4 projection_matrix = glm::ortho(0.0f, (float)mode->width, (float)mode->height, 0.0f, -1.0f, 1.0f);
    
	// Texture
	Texture* texture_obj;
    Shader* shader_texture = new Shader("../../Common/shaders/textured_triangle.glsl");;
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

	float increment = 0.01f;
	glm::vec3 sheet_pos(0, 0.0f, 0.0f);
	glm::vec3 model_a_pos(0, 0.0f, 0.0f);
	glm::vec3 model_b_pos(mode->width/ 2.0f - mode->width/16.0f, mode->height / 2.0f - mode->width / 16.0f, 0.0f);
	glm::vec3 camera_pos(0.0f);
	ImGuiColorEditFlags f = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;
    // Main loop
    while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

        new_imgui_frame();

		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");

		// Move the model up (means closer to 0 in content coordinates)
		ImGui::SliderFloat2("Model A Coordinates", &model_a_pos.x, 0.0f - 500, (float)mode->width, "%.1f", 1.0f);
		ImGui::SliderFloat2("Model B Coordinates", &model_b_pos.x, 0.0f - 500, (float)mode->width, "%.1f", 1.0f);
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

        // View matrix
		glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), -camera_pos);

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

        // draw calls for each separate translations
        std::vector<glm::vec3*> model_positions = { &model_a_pos, &model_b_pos };
        std::vector<float*> model_colors = { &(triangle_color_a[0]), &(triangle_color_b[0])};
        for (int i = 0; i < 2; i++)
		{
            auto model_i_pos = model_positions[i];
            auto model_i_color = model_colors[i];
			// Compute MVP, view translations are reversed to mimic a camera
			glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), *model_i_pos);
			glm::mat4 MVP_matrix = projection_matrix * view_matrix * model_matrix;

			// Update locations and colors
			shader_raw->bind();
			shader_raw->set_uniform_4f("u_color",
				model_i_color[0],
				model_i_color[1],
				model_i_color[2],
				model_i_color[3]);
			shader_raw->set_uniform_mat4f("u_MVP", MVP_matrix);
            shader_texture->bind();
			shader_texture->set_uniform_mat4f("u_MVP", MVP_matrix);

			// Draw call
            if (has_texture)
            {
				renderer.draw(vertex_array_obj, index_buffer_obj, shader_texture);
            }
            else
			{
				renderer.draw(vertex_array_obj, index_buffer_obj, shader_raw);
            }
        }

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
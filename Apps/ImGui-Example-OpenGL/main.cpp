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
    #define has_texture true
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

    // Enable blending
    __glCallVoid(glEnable(GL_BLEND));
    __glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA));

	// Init vertex array object
    VertexArray* vertex_array_obj = new VertexArray;

	// Create the vertex buffer
    VertexBuffer* vertex_buffer_obj = new VertexBuffer(positions, num_vertices*num_coord_p_vertex *sizeof(float));
    VertexBufferLayout* vertex_buffer_layout = new VertexBufferLayout();
	vertex_buffer_layout->push_back_elements<float>(num_coordinates);
	if (has_texture) vertex_buffer_layout->push_back_elements<float>(num_coordinates);
    vertex_array_obj->add_buffer(*vertex_buffer_obj, *vertex_buffer_layout);

	// Create index buffer
	IndexBuffer* index_buffer_obj = new IndexBuffer(indices, num_indices);

	// Compile & bind shaders
    Shader* shader_obj = new Shader("../../Common/shaders/triangle.glsl");

    // Project into window content coordinate system
    glm::mat4 projection_matrix = glm::ortho(0.0f, (float)mode->width, (float)mode->height, 0.0f, -1.0f, 1.0f);
    
	// Texture
    Texture* texture_obj;
    if (has_texture)
	{
		texture_obj = new Texture("../../Data/textures/eye.png");
		texture_obj->bind();
		shader_obj->set_uniform_1i("u_texture", 0);
    }
    
	// Specify the color of the triangle
	float triangle_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    // Unbind all, as no longer binding needed
    vertex_array_obj->unbind();
    vertex_buffer_obj->unbind();
	index_buffer_obj->unbind();
	shader_obj->unbind();

    Renderer renderer;

	float increment = 0.01f;
	glm::vec3 model_a_pos(0, 0.0f, 0.0f);
	glm::vec3 model_b_pos(mode->width/ 2.0f - mode->width/16.0f, mode->height / 2.0f - mode->width / 16.0f, 0.0f);
	glm::vec3 camera_pos(0.0f);
    // Main loop
    while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

        new_imgui_frame();

		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");

		// Move the model up (means closer to 0 in content coordinates)
		ImGui::SliderFloat2("Model A Coordinates", &model_a_pos.x, 0.0f, mode->width, "%.1f", 1.0f);
		ImGui::SliderFloat2("Model B Coordinates", &model_b_pos.x, 0.0f, mode->width, "%.1f", 1.0f);
        ImGui::SliderFloat2("Camera Coordinates", &camera_pos.x, mode->width / -2.0f, mode->width / 2.0f, "%.3f", 1.0f);

		ImGui::ColorEdit3("clear color", (float*)&clear_color);

		//ImGui::ColorEdit4("uniform color", (float*)&triangle_color);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
            1000.0f / ImGui::GetIO().Framerate, 
            ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::EndFrame();

        // Clear background
		renderer.set_viewport(window);
        renderer.clear((float*)&clear_color);

		glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), -camera_pos);

        // draw calls for each separate translations
        for (const auto& model_positions = {model_a_pos, model_b_pos}; const auto& model_i_pos : model_positions)
		{
			// Compute MVP, view translations are reversed to mimic a camera
			glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), model_i_pos);
			glm::mat4 MVP_matrix = projection_matrix * view_matrix * model_matrix;

			// Update locations and colors
			shader_obj->bind();
			shader_obj->set_uniform_mat4f("u_MVP", MVP_matrix);

			// Draw call
			renderer.draw(vertex_array_obj, index_buffer_obj, shader_obj);
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
    delete shader_obj;
    if (has_texture) delete texture_obj;

    // Cleanup
    shutdown_imgui(cfg);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
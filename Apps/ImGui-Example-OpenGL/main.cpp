#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glew.h"
#include "glfw3.h"
#include "Common/ShaderManager.h"
#include "Common/ErrorManager.h"
#include "Common/VertexBuffer.h"
#include "Common/IndexBuffer.h"
#include "Common/VertexArray.h"
#include "Common/VertexBufferLayout.h"
#include <stdio.h>
#include <iostream>
#include <string>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
#include <Common/ImGuiManager.h>

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
    if (window == NULL)
	{
		glfwTerminate();
		return -1;
    }
    glfwMakeContextCurrent(window);
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
    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    // The triangles
    constexpr unsigned int num_vertices = 4;
	constexpr unsigned int num_coordinates = 2;
	constexpr unsigned int num_indices = 6;
    float positions[num_vertices * num_coordinates] = {
		 -0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f
    };
	unsigned int indices[num_indices] = {
		0, 1, 2,
		0, 2, 3
	};

	// Init vertex array object
    VertexArray* vertex_array_obj = new VertexArray;

	// Create the vertex buffer
    VertexBuffer* vertex_buffer_obj = new VertexBuffer(positions, num_vertices*num_coordinates*sizeof(float));
    VertexBufferLayout* vertex_buffer_layout = new VertexBufferLayout();
    vertex_buffer_layout->push_back_elements<float>(num_coordinates);
    vertex_array_obj->add_buffer(*vertex_buffer_obj, *vertex_buffer_layout);

	// Create index buffer
	IndexBuffer* index_buffer_obj = new IndexBuffer(indices, num_indices);

	// Compile & bind shaders
	unsigned int program_id = compile_and_bind_shader("../../Common/shaders/triangle.glsl");
    
	// Specify the color of the triangle
	unsigned int uniform_location;
	float triangle_color[4] = { 0.3f, 0.2f, 1.0f, 1.0f };
	__glCallReturn(glGetUniformLocation(program_id, "u_color"), uniform_location);
	ASSERT(uniform_location != -1);
	__glCallVoid(glUniform4f(uniform_location,
		triangle_color[0],
		triangle_color[1],
		triangle_color[2],
		triangle_color[3]));
    
    // Unbind all buffers, as no longer binding needed
    vertex_array_obj->unbind();
    __glCallVoid(glUseProgram(0));
    vertex_buffer_obj->unbind();
    index_buffer_obj->unbind();

	float increment = 0.01f;
    // Main loop
    while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

        new_imgui_frame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");
		ImGui::Checkbox("Demo Window", &show_demo_window);

		ImGui::ColorEdit3("clear color", (float*)&clear_color);

		ImGui::ColorEdit4("uniform color", (float*)&triangle_color);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
            1000.0f / ImGui::GetIO().Framerate, 
            ImGui::GetIO().Framerate);
		ImGui::End();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        __glCallVoid(glViewport(0, 0, display_w, display_h));
        __glCallVoid(glClearColor(clear_color.x * clear_color.w,
            clear_color.y * clear_color.w, 
            clear_color.z * clear_color.w, 
            clear_color.w));
        __glCallVoid(glClear(GL_COLOR_BUFFER_BIT));

		__glCallVoid(glUseProgram(program_id));
		__glCallVoid(glUniform4f(uniform_location,
			triangle_color[0],
			triangle_color[1],
			triangle_color[2],
			triangle_color[3]));

        vertex_array_obj->bind();
        index_buffer_obj->bind();

		// Draw triangle(s) behind the ImGui
        index_buffer_obj->draw();

		// Always draw ImGui on top of the app
        render_imgui();

		// Square color animation
		if (triangle_color[2] >= 1.0f)
		{
			increment *= -1;
		}
		else if (triangle_color[2] <= 0.0f)
		{
			increment *= -1;
		}
		triangle_color[2] += increment;

        glfwSwapBuffers(window);
        
    }

    // delete for each new
    delete vertex_buffer_obj;
    delete index_buffer_obj;
    delete vertex_array_obj;
    delete vertex_buffer_layout;

    // Cleanup
    shutdown_imgui(cfg);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
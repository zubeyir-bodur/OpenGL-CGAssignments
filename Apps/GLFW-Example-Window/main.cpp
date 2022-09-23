#include "glew.h"
#include "glfw3.h"
#include "Common/ShaderManager.h"
#include "Common/Draw.h"
#include "Common/ErrorManager.h"

#include <iostream>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    int display_w = 640, display_h = 480;
    float clear_color[] = { 0.5f, 1.0f, 0.0f, 0.5f };
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(display_w, display_h, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Init GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Could not init GLEW..." << std::endl;
    }
    else
    {
		const unsigned char* opengl_version;
		glCallReturn(glGetString(GL_VERSION), opengl_version);
		std::cout << "OpenGL version: " << opengl_version << std::endl;

		const unsigned char* glsl_version;
		glCallReturn(glGetString(GL_SHADING_LANGUAGE_VERSION), glsl_version);
		std::cout << "GLSL version: " << glsl_version << std::endl;
    }

    // The triangles
    unsigned int vertex_buffer_id;
    constexpr int num_vertices = 4;
    constexpr int num_coordinates_per_vertex = 2;
    constexpr unsigned int num_indices = 6;
    constexpr int start_index = 0;
    float vertex_buffer[num_vertices * num_coordinates_per_vertex] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f
    };
    unsigned int index_buffer[num_indices] = {
        0, 1, 2,
        0, 2, 3
    };

    // Create the vertex buffer
    vertex_buffer_id = set_up_vertex_buffer(vertex_buffer, num_vertices, num_coordinates_per_vertex);

	// Create index buffer
	unsigned int index_buffer_id = set_up_index_buffer(index_buffer, num_indices);

    // Compile & bind shaders
	unsigned int program_id = compile_and_bind_shader("../../Common/shaders/triangle.glsl");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		/* Background draw*/
		glCallVoid(glViewport(0, 0, display_w, display_h));
		glCallVoid(glClearColor(clear_color[0] * clear_color[3],
            clear_color[1] * clear_color[3],
            clear_color[2] * clear_color[3],
            clear_color[3]));
        glCallVoid(glClear(GL_COLOR_BUFFER_BIT));

        // Draw triangle(s)
        glCallVoid(glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr));

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    shutdown(program_id, vertex_buffer_id, index_buffer_id);
    glfwTerminate();
    return 0;
}

#include <iostream>
#include "Common/shader_manager.h"
#include "glew.h" // always include before glfw, which includes GL.h
#include "glfw3.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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
        std::cout << "OpenGL version: "  << glGetString(GL_VERSION) << std::endl;
    }

    // The triangle
    unsigned int buffer_id;
    constexpr int num_vertices = 3;
    constexpr int num_coordinates_per_vertex = 2;
    constexpr int start_index = 0;
    float positions[num_vertices * num_coordinates_per_vertex] = {
        -0.5f, -0.5f,
        0.0f, 0.5f,
        0.5f, -0.5f
    };

    // Setting up the triangle for OpenGL
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * 2 * sizeof(float), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(start_index);
    glVertexAttribPointer(start_index,
        num_coordinates_per_vertex,
        GL_FLOAT, //type
        GL_FALSE, // normalized flag
        sizeof(float) * num_coordinates_per_vertex, // stride size, in bytes
        nullptr // pointer to the next attribute
    );

    std::string program_src = parse_shader_file("../../Common/shaders/triangle.glsl");
    unsigned int program_id = create_program_from_shaders(program_src);
    glUseProgram(program_id);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw triangle(s)
        glDrawArrays(GL_TRIANGLES, start_index, num_vertices);

        /*
        // Legacy OpenGL code that draws a triangle
    	glBegin(GL_TRIANGLES);

        glVertex2f(-0.5f, -0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(0.5f, -0.5f);

        glEnd();*/

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteProgram(program_id);
    glDeleteBuffers(1, &buffer_id);
    glfwTerminate();
    return 0;
}

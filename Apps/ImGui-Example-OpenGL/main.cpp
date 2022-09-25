#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glew.h"
#include "glfw3.h"
#include "Common/ShaderManager.h"
#include "Common/Draw.h"
#include "Common/ErrorManager.h"

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
        return 1;

    // Inputting 3.0 successfully attempts to get the highest OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create GLFW full screen window
    GLFWmonitor* main_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(main_monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer back ends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	std::string glsl_version_std = "#version " + std::to_string(get_glsl_version()) + " core";
	const char* glsl_version = glsl_version_std.c_str();
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Default Font
    ImFontConfig* cfg = new ImFontConfig();
    cfg->SizePixels = 20;
    io.Fonts->AddFontDefault(cfg);

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

    // ImGui state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

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

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Needed for ImGui

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          

            ImGui::Text("This is some useful text.");               
            ImGui::Checkbox("Demo Window", &show_demo_window);      
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            
            ImGui::ColorEdit3("clear color", (float*)&clear_color); 

            if (ImGui::Button("Button"))                            
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glCallVoid(glViewport(0, 0, display_w, display_h));
        glCallVoid(glClearColor(clear_color.x * clear_color.w,
            clear_color.y * clear_color.w, 
            clear_color.z * clear_color.w, 
            clear_color.w));
        glCallVoid(glClear(GL_COLOR_BUFFER_BIT));

		// Draw triangle(s) behind the ImGui
        glCallVoid(glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr));

        // Always draw ImGui on top of the app
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	shutdown(program_id, vertex_buffer_id, index_buffer_id);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
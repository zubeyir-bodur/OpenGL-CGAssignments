#include "ImGuiManager.h"
#include "dearimgui/imgui.h"
#include "dearimgui/backend/imgui_impl_glfw.h"
#include "dearimgui/backend/imgui_impl_opengl3.h"
#include "Shader.h"

ImFontConfig* init_imgui(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer back ends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	std::string glsl_version_std = "#version " + std::to_string(Shader::get_glsl_version()) + " core";
	const char* glsl_version = glsl_version_std.c_str();
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Default Font
	ImFontConfig* cfg = new ImFontConfig();
	cfg->SizePixels = 20;
	io.Fonts->AddFontDefault(cfg);
	return cfg;
}

void new_imgui_frame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void render_imgui()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdown_imgui(ImFontConfig* cfg)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	delete cfg;
}

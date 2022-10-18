#pragma once
struct ImFontConfig;
struct GLFWwindow;

void init_imgui(GLFWwindow* window);
void new_imgui_frame();
void render_imgui();
void shutdown_imgui();
void SetupImGuiStyle();
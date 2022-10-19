#pragma once
#include <glfw3.h>
#include <cstring>
#include "Angel-maths/mat.h"

class Input
{
private:

	GLFWwindow* m_app_window;
	Angel::mat4* m_proj, * m_view;

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
public:
	enum class ButtonState
	{
		Idle = -1,
		JustPressed = GLFW_PRESS,
		BeingPressed = 2,
		Released = GLFW_RELEASE,
	};

	ButtonState m_lmb_state;

	double m_mouse_x, m_mouse_y;
	double m_mouse_press_x, m_mouse_press_y;
	double m_mouse_release_x, m_mouse_release_y;
	double m_scroll_y;

	// Singleton constructors and instance function
	Input(GLFWwindow* app_window);
	Input(const Input&) = delete;

	static Input& get_instance(GLFWwindow* app_window);
};
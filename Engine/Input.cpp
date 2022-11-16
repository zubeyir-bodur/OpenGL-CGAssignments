#include "Input.h"
#include <iostream>
#include <string>
#include "Angel-maths/mat.h"

Input::Input(GLFWwindow* app_window)
{
	m_app_window = app_window;
	m_mouse_x = -1.0;
	m_mouse_y = -1.0;
	m_lmb_state = ButtonState::Idle;
	m_rmb_state = ButtonState::Idle;
	glfwSetScrollCallback(app_window, Input::mouse_scroll_callback);
	glfwSetCursorPosCallback(app_window, Input::mouse_move_callback);
	glfwSetMouseButtonCallback(app_window, Input::mouse_button_callback);
	glfwSetKeyCallback(app_window, Input::keyboard_callback);
}

Input& Input::get_instance(GLFWwindow* app_window)
{
	static Input instance(app_window);
	return instance;
}

void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Input& instance = Input::get_instance(window);
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			instance.m_lmb_state = ButtonState::JustPressed;
			glfwGetCursorPos(window, &instance.m_mouse_press_x, &instance.m_mouse_press_y);
		}
		else if (action == GLFW_RELEASE)
		{
			instance.m_lmb_state = ButtonState::Released;
			glfwGetCursorPos(window, &instance.m_mouse_release_x, &instance.m_mouse_release_y);
		}
		else
		{
			instance.m_lmb_state = ButtonState::Idle;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			instance.m_rmb_state = ButtonState::JustPressed;
			glfwGetCursorPos(window, &instance.m_mouse_press_x, &instance.m_mouse_press_y);
		}
		else if (action == GLFW_RELEASE)
		{
			instance.m_rmb_state = ButtonState::Released;
			glfwGetCursorPos(window, &instance.m_mouse_release_x, &instance.m_mouse_release_y);
		}
		else
		{
			instance.m_rmb_state = ButtonState::Idle;
		}
	}
}

void Input::mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	Input& instance = Input::get_instance(window);
	instance.m_mouse_x = xpos;
	instance.m_mouse_y = ypos;
}

void Input::mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Input& instance = Input::get_instance(window);
	instance.m_scroll_y = yoffset;
}

void Input::keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Input& instance = Input::get_instance(window);
	if (mods & GLFW_MOD_CONTROL)
	{
		if (key == GLFW_KEY_C && action == GLFW_PRESS)
		{
			instance.m_copy_just_pressed = true;
		}
		if (key == GLFW_KEY_V && action == GLFW_PRESS)
		{
			instance.m_paste_just_pressed = true;
		}
	}
}


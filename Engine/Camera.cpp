#include "Camera.h"

Camera::Camera(){}
Camera::~Camera(){}

void Camera::init(const Angel::vec3& pos, float zoom_ratio)
{
	Camera& c = get_instance();
	c.m_camera_pos = pos;
	c.m_zoom_ratio = zoom_ratio;
}

Camera& Camera::get_instance()
{
	static Camera c;
	return c;
}

void Camera::move_vertical(float dy)
{
	Camera& c = get_instance();
	c.m_camera_pos.y += dy;
}

void Camera::move_horizontal(float dx)
{
	Camera& c = get_instance();
	c.m_camera_pos.x += dx;
}

void Camera::zoom(double zoom_percent_delta, double global_cursor_x, double global_cursor_y)
{
	const Angel::vec3& cursor_model_coords = map_from_global(global_cursor_x, global_cursor_y);
	Camera& c = get_instance();
	// Camera zooming with mouse wheel
	if (c.m_zoom_ratio >= 20 && c.m_zoom_ratio <= 1000) // min 20 % max 10x zoom
	{
		if (zoom_percent_delta != 0)
		{
			c.m_zoom_ratio += (float)zoom_percent_delta * 10.0f;
		}
	}
	// Avoid exceeding limits
	if (c.m_zoom_ratio < 20.0)
	{
		c.m_zoom_ratio = 20.0;
	}
	if (c.m_zoom_ratio > 1000.0)
	{
		c.m_zoom_ratio = 1000.0;
	}
	// Move the camera to the mouse location
	if (c.m_zoom_ratio >= 20 && c.m_zoom_ratio <= 1000)
	{
		if (zoom_percent_delta != 0)
		{
			c.m_camera_pos -= (c.m_zoom_ratio / 100.0f) * ((c.m_camera_pos + Angel::vec3((float)global_cursor_x, (float)global_cursor_y, 0.0f)) * (100.0f / c.m_zoom_ratio) - cursor_model_coords);
		}
	}
}

const Angel::vec3 Camera::camera_pos()
{
	Camera& c = get_instance();
	return Angel::vec3(c.m_camera_pos);
}

Angel::mat4 Camera::view_matrix()
{
	Camera& c = get_instance();
	return  Angel::Translate(-c.m_camera_pos)
		* Angel::Scale(Angel::vec3(c.m_zoom_ratio / 100.0f, c.m_zoom_ratio / 100.0f, 1.0f));
}

Angel::vec3 Camera::map_from_global(double x, double y)
{
	Camera& c = get_instance();
	return (c.m_camera_pos + Angel::vec3((float)x, (float)y, 0.0f))* (100.0f / c.m_zoom_ratio);
}

const float& Camera::get_zoom_ratio()
{
	Camera& c = get_instance();
	return c.m_zoom_ratio;
}

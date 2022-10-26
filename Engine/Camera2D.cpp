#include "Camera2D.h"
#define MIN_ZOOM 50.0f
#define MAX_ZOOM 1000.0f

Camera2D::Camera2D(){}
Camera2D::~Camera2D(){}

void Camera2D::init(const Angel::vec3& pos, float zoom_ratio)
{
	Camera2D& c = get_instance();
	c.m_camera_pos = pos;
	c.m_zoom_ratio = zoom_ratio;
}

Camera2D& Camera2D::get_instance()
{
	static Camera2D c;
	return c;
}

void Camera2D::move_vertical(float dy)
{
	Camera2D& c = get_instance();
	c.m_camera_pos.y += dy;
}

void Camera2D::move_horizontal(float dx)
{
	Camera2D& c = get_instance();
	c.m_camera_pos.x += dx;
}

void Camera2D::zoom(double zoom_percent_delta, double global_cursor_x, double global_cursor_y)
{
	const Angel::vec3& cursor_world_before_zoom = map_from_global(global_cursor_x, global_cursor_y);
	Camera2D& c = get_instance();
	// Camera zooming with mouse wheel
	if (c.m_zoom_ratio >= MIN_ZOOM && c.m_zoom_ratio <= MAX_ZOOM) // min 20 % max 10x zoom
	{
		if (zoom_percent_delta != 0)
		{
			c.m_zoom_ratio += (float)zoom_percent_delta * 10.0f;
		}
	}
	// Avoid exceeding limits
	if (c.m_zoom_ratio < MIN_ZOOM)
	{
		c.m_zoom_ratio = MIN_ZOOM;
	}
	if (c.m_zoom_ratio > MAX_ZOOM)
	{
		c.m_zoom_ratio = MAX_ZOOM;
	}
	// Move the camera to the mouse location
	if (c.m_zoom_ratio >= MIN_ZOOM && c.m_zoom_ratio <= MAX_ZOOM)
	{
		if (zoom_percent_delta != 0)
		{
			c.m_camera_pos -= (c.m_zoom_ratio / 100.0f) * (map_from_global(global_cursor_x, global_cursor_y) - cursor_world_before_zoom);
		}
	}
}

const Angel::vec3 Camera2D::camera_pos()
{
	Camera2D& c = get_instance();
	return Angel::vec3(c.m_camera_pos);
}

Angel::mat4 Camera2D::view_matrix()
{
	Camera2D& c = get_instance();
	return  Angel::Translate(-c.m_camera_pos)
		* Angel::Scale(Angel::vec3(c.m_zoom_ratio / 100.0f, c.m_zoom_ratio / 100.0f, 1.0f));
}

Angel::vec3 Camera2D::map_from_global(double x, double y)
{
	Camera2D& c = get_instance();
	return (c.m_camera_pos + Angel::vec3((float)x, (float)y, 0.0f))* (100.0f / c.m_zoom_ratio);
}

Angel::vec3 Camera2D::map_to_global(const Angel::vec3& model)
{
	Camera2D& c = get_instance();
	return Angel::vec3((-c.m_camera_pos + model) * (c.m_zoom_ratio/ 100.0f));
}

const float& Camera2D::get_zoom_ratio()
{
	Camera2D& c = get_instance();
	return c.m_zoom_ratio;
}

#include "OrthogtraphicCamera.h"
#define MIN_ZOOM 50.0f
#define MAX_ZOOM 1000.0f

void OrthogtraphicCamera::init(const Angel::vec3& pos, float zoom_ratio)
{
	OrthogtraphicCamera& c = get_instance();
	c.m_camera_pos = pos;
	c.m_zoom_ratio = zoom_ratio;
	c.m_should_update = false;
}

OrthogtraphicCamera& OrthogtraphicCamera::get_instance()
{
	static OrthogtraphicCamera c;
	return c;
}

void OrthogtraphicCamera::move_vertical(float dy)
{
	OrthogtraphicCamera& c = get_instance();
	c.m_camera_pos.y += dy;
	c.m_should_update = true;
}

void OrthogtraphicCamera::move_horizontal(float dx)
{
	OrthogtraphicCamera& c = get_instance();
	c.m_camera_pos.x += dx;
	c.m_should_update = true;
}

void OrthogtraphicCamera::zoom_camera_towards(double zoom_percent_delta, double global_cursor_x, double global_cursor_y)
{
	const Angel::vec3& cursor_world_before_zoom = map_from_global(global_cursor_x, global_cursor_y);
	OrthogtraphicCamera& c = get_instance();
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
	c.m_should_update = true;
}

const Angel::vec3& OrthogtraphicCamera::position()
{
	OrthogtraphicCamera& c = get_instance();
	return c.m_camera_pos;
}


const Angel::mat4& OrthogtraphicCamera::view_matrix()
{
	OrthogtraphicCamera& c = get_instance();
	return  c.m_view_matrix;
}

void OrthogtraphicCamera::update()
{
	OrthogtraphicCamera& c = get_instance();
	if (c.m_should_update)
	{
		c.m_view_matrix = Angel::Translate(-c.m_camera_pos)
			* Angel::Scale(Angel::vec3(c.m_zoom_ratio / 100.0f, c.m_zoom_ratio / 100.0f, 1.0f));
		c.m_should_update = false;
	}
}

Angel::vec3 OrthogtraphicCamera::map_from_global(double x, double y)
{
	OrthogtraphicCamera& c = get_instance();
	return (c.m_camera_pos + Angel::vec3((float)x, (float)y, 0.0f))* (100.0f / c.m_zoom_ratio);
}

Angel::vec3 OrthogtraphicCamera::map_to_global(const Angel::vec3& model)
{
	OrthogtraphicCamera& c = get_instance();
	return Angel::vec3((-c.m_camera_pos + model) * (c.m_zoom_ratio/ 100.0f));
}

const float& OrthogtraphicCamera::zoom_ratio()
{
	OrthogtraphicCamera& c = get_instance();
	return c.m_zoom_ratio;
}

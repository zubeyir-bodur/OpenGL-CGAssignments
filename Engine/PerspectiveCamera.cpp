#include "PerspectiveCamera.h"

PerspectiveCamera& PerspectiveCamera::get_instance()
{
	static PerspectiveCamera c;
	return c;
}

void PerspectiveCamera::init(const Angel::vec3& position, float fovy, float aspect_ratio)
{
	PerspectiveCamera& c = get_instance();
	c.m_position = position;
	c.m_fovy = fovy;
	c.m_aspect_ratio = aspect_ratio;
	c.m_proj_matrix = Angel::Perspective(c.m_fovy, c.m_aspect_ratio, 0.1f, 1e+38f);
	c.m_view_matrix = Angel::Rotate(c.m_rotation) * Angel::Translate(-c.m_position);
	c.m_should_update = false;
}

/// <summary>
/// Moves the camera up regardless of the camera direction
/// </summary>
/// <param name="dy"></param>
void PerspectiveCamera::pedestal(float dy)
{
	PerspectiveCamera& c = get_instance();

	// Do stuff
	c.m_position.y += dy;
	c.m_should_update = true;
}

/// <summary>
/// Left and right, relative to the camera direction
/// </summary>
/// <param name="delta"></param>
void PerspectiveCamera::truck(float delta)
{
	PerspectiveCamera& c = get_instance();

	// Do stuff
	Angel::vec3 tmp = Angel::XDir(c.m_rotation) * delta;
	c.m_position.z -= tmp.z;
	c.m_position.x += tmp.x;
	c.m_position.y += tmp.y;
	c.m_should_update = true;
}

/// <summary>
/// Forward Backward, relative to the camera direction
/// </summary>
/// <param name="dz"></param>
void PerspectiveCamera::dolly(float delta)
{
	PerspectiveCamera& c = get_instance();

	// Do stuff
	Angel::vec3 tmp = Angel::ZDir(c.m_rotation) * delta;
	c.m_position.z -= tmp.z;
	c.m_position.x += tmp.x;
	c.m_position.y += tmp.y;
	c.m_should_update = true;
}

/// <summary>
/// Left/Right rotation, happens if rotated around Y axis
/// </summary>
/// <param name="d_theta_y">
/// Y angle to accumulate, which comes from dragging from left to right in the trackball
/// </param>
void PerspectiveCamera::pan(float d_theta_y)
{
	PerspectiveCamera& c = get_instance();

	// Do stuff - TODO implement
	c.m_rotation = c.m_rotation * Angel::Quat({ 0.0f, 1.0f, 0.0f }, d_theta_y);
	c.m_should_update = true;
}

/// <summary>
/// Up down rotation relative to the camera, happens if rotated around X axis
/// </summary>
/// <param name="d_thetax">
/// X angle to accumulate, which comes from dragging from up to down in the trackball
/// </param>
void PerspectiveCamera::tilt(float d_theta_y)
{
	PerspectiveCamera& c = get_instance();

	// Do stuff - TODO implement
	c.m_rotation = Angel::Quat({ 1.0f, 0.0f, 0.0f }, d_theta_y) * c.m_rotation;
	c.m_should_update = true;
}

void PerspectiveCamera::roll(float d_thetaz)
{
	PerspectiveCamera& c = get_instance();

	// Do stuff
	c.m_rotation = c.m_rotation * Angel::Quat({ 0.0f, 0.0f, 1.0f }, d_thetaz);
	c.m_should_update = true;
}

void PerspectiveCamera::fovy(float new_fovy)
{
	PerspectiveCamera& c = get_instance();
	c.m_fovy = new_fovy;
	c.m_should_update = true;
}

void PerspectiveCamera::resize(float new_aspect_ratio)
{
	PerspectiveCamera& c = get_instance();
	c.m_aspect_ratio = new_aspect_ratio;
	c.m_should_update = true;
}

const Angel::mat4& PerspectiveCamera::view_matrix()
{
	PerspectiveCamera& c = get_instance();
	return c.m_view_matrix;
}

const Angel::mat4& PerspectiveCamera::proj_matrix()
{
	PerspectiveCamera& c = get_instance();
	return c.m_proj_matrix;
}

Angel::vec3& PerspectiveCamera::position()
{
	PerspectiveCamera& c = get_instance();
	c.m_should_update = true;
	return c.m_position;
}

Angel::quat& PerspectiveCamera::rotation()
{
	PerspectiveCamera& c = get_instance();
	c.m_should_update = true;
	return c.m_rotation;
}

void PerspectiveCamera::update()
{
	PerspectiveCamera& c = get_instance();
	if (c.m_should_update)
	{
		c.m_proj_matrix = Angel::Perspective(c.m_fovy, c.m_aspect_ratio, 0.1f, 1e+38f);
		c.m_view_matrix = Angel::Rotate(c.m_rotation) * Angel::Translate(-c.m_position);
		c.m_should_update = false;
	}
}


#include "Camera/PerspectiveCamera.h"

/// <summary>
/// FIX-ME-OPT: Update the view matrix once per frame only when the camera is moved or rotated
/// </summary>
void PerspectiveCamera::update()
{
	m_camera_front.x = cos(Angel::radians(m_yaw)) * cos(Angel::radians(m_pitch));
	m_camera_front.y = sin(Angel::radians(m_pitch));
	m_camera_front.z = sin(Angel::radians(m_yaw)) * cos(Angel::radians(m_pitch));

	m_camera_front = Angel::normalize(m_camera_front);
	m_camera_right = Angel::normalize(Angel::cross(m_camera_front, m_world_up));
	m_camera_up = Angel::normalize(Angel::cross(m_camera_right, m_camera_front)); 

	// Only update the view matrix
	m_view_matrix = Angel::LookAt(m_camera_position, m_camera_position + m_camera_front, m_camera_up);
}

PerspectiveCamera& PerspectiveCamera::instance()
{
	static PerspectiveCamera c;
	return c;
}


void PerspectiveCamera::init(Angel::vec3 position, Angel::vec3 world_up, float fov, int width, int height)
{
	PerspectiveCamera& c = instance();

	// Store the initial world up value
	c.m_world_up = world_up; 

	// Viewport
	c.m_viewport_width = width;
	c.m_viewport_height = height;

	// Initial camera position & Fov
	c.m_camera_position = position;
	c.m_fovy = fov;

	// Set up the projection matrix
	c.m_proj_matrix = Angel::Perspective(c.m_fovy, c.m_viewport_width / (float)c.m_viewport_height, c.m_z_near, c.m_z_far);

	// Pseudo state look at vectors, to be updated in the update function
	c.m_camera_up = c.m_world_up;
	c.update();
}

const Angel::mat4& PerspectiveCamera::view_matrix()
{
	PerspectiveCamera& c = instance();
	return c.m_view_matrix;
}

const Angel::mat4& PerspectiveCamera::projection_matrix()
{
	PerspectiveCamera& c = instance();
	return c.m_proj_matrix;
}

const Angel::vec3& PerspectiveCamera::position()
{
	PerspectiveCamera& c = instance();
	return c.m_camera_position;
}

const float& PerspectiveCamera::pitch()
{
	PerspectiveCamera& c = instance();
	return c.m_pitch;
}

const float& PerspectiveCamera::yaw()
{
	PerspectiveCamera& c = instance();
	return c.m_yaw;
}

const float& PerspectiveCamera::roll()
{
	PerspectiveCamera& c = instance();
	return c.m_roll;
}

void PerspectiveCamera::move(const float& dt_seconds, const MovementDirection direction)
{
	PerspectiveCamera& c = instance();
	//Update m_position vector
	switch (direction)
	{
	case MovementDirection::FORWARD:
		c.m_camera_position += c.m_camera_front * c.m_movement_speed * dt_seconds;
		break;
	case MovementDirection::BACKWARD:
		c.m_camera_position -= c.m_camera_front * c.m_movement_speed * dt_seconds;
		break;
	case MovementDirection::LEFT:
		c.m_camera_position -= c.m_camera_right * c.m_movement_speed * dt_seconds;
		break;
	case MovementDirection::RIGHT:
		c.m_camera_position += c.m_camera_right * c.m_movement_speed * dt_seconds;
		break;
	case MovementDirection::UP:
		c.m_camera_position += c.m_world_up * c.m_movement_speed * dt_seconds; // Elevates the camera in the specified world up direction
		break;
	case MovementDirection::DOWN:
		c.m_camera_position -= c.m_world_up * c.m_movement_speed * dt_seconds;
		break;
	default:
		break;
	}
	c.update(); // Compute the multiplications iff there is a change
}

void PerspectiveCamera::rotate(const float& dt_seconds, const double& offset_x, const double& offset_y)
{
	PerspectiveCamera& c = instance();
	//Update m_pitch and m_yaw
	c.m_pitch -= static_cast<float>(offset_y) * c.m_rotation_sensitivity/* * dt_seconds*/; // Bug-fix for fast camera rotation when FPS is low
	c.m_yaw += static_cast<float>(offset_x) * c.m_rotation_sensitivity/* * dt_seconds*/;

	// FIX-ME-OPT: use proper quaternions to fix the gimball lock
	//if (c.m_pitch > 89.999f && c.m_pitch <= 90.0f)
	//{
	//	c.m_pitch = 90.001f;
	//}
	//else if (c.m_pitch < -89.999f && c.m_pitch >= -90.0f)
	//{
	//	c.m_pitch = -90.001f;
	//}
	// 	if (c.m_pitch >= 360.0f)
	// 	{
	// 		c.m_pitch = 0.0f;
	// 	}
	// 	else if (c.m_pitch <= -360.0f)
	// 	{
	// 		c.m_pitch = 0.0f;
	// 	}
	if (c.m_pitch >= 89.0f)
	{
		c.m_pitch = 89.0f;
	}
	else if (c.m_pitch <= -89.0f)
	{
		c.m_pitch = -89.0f;
	}
	// Prevent yaw from accumulating
	if (c.m_yaw > 360.f || c.m_yaw < -360.f)
	{
		c.m_yaw = 0.f;
	}
	c.update(); // Compute the multiplications iff there is a change
}

void PerspectiveCamera::on_viewport_resize(int width, int height)
{
	PerspectiveCamera& c = instance();
	if (c.m_viewport_width != width 
		|| c.m_viewport_height != height)
	{
		c.m_viewport_width = width;
		c.m_viewport_height = height;
		// Only update the proj matrix
		c.m_proj_matrix = Angel::Perspective(c.m_fovy, c.m_viewport_width / (float)c.m_viewport_height, c.m_z_near, c.m_z_far);
	}
}

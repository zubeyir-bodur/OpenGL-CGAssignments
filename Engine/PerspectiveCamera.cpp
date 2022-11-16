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
	c.m_view_matrix = Angel::rotate(c.m_rotation) * Angel::Translate(-c.m_position);
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
	Angel::vec3 tmp = Angel::x_dir(c.m_rotation) * delta;
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
	Angel::vec3 tmp = Angel::z_dir(c.m_rotation) * delta;
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
	c.m_rotation = c.m_rotation * Angel::quat_from_axis_angle({ 0.0f, 1.0f, 0.0f }, d_theta_y);
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
	c.m_rotation = Angel::quat_from_axis_angle({ 1.0f, 0.0f, 0.0f }, d_theta_y) * c.m_rotation;
	c.m_should_update = true;
}

void PerspectiveCamera::roll(float d_thetaz)
{
	PerspectiveCamera& c = get_instance();

	// Do stuff
	c.m_rotation = c.m_rotation * Angel::quat_from_axis_angle({ 0.0f, 0.0f, 1.0f }, d_thetaz);
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
		c.m_view_matrix = Angel::rotate(c.m_rotation) * Angel::Translate(-c.m_position);
		c.m_should_update = false;
	}
}

/// <summary>
/// FIX-ME-OPT: Update the view matrix once per frame only when the camera is moved or rotated
/// </summary>
void Camera::update()
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

Camera& Camera::instance()
{
	static Camera c;
	return c;
}


void Camera::init(Angel::vec3 position, Angel::vec3 world_up, float fov, int width, int height)
{
	Camera& c = instance();

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

const Angel::mat4& Camera::view_matrix()
{
	Camera& c = instance();
	return c.m_view_matrix;
}

const Angel::mat4& Camera::projection_matrix()
{
	Camera& c = instance();
	return c.m_proj_matrix;
}

const Angel::vec3& Camera::position()
{
	Camera& c = instance();
	return c.m_camera_position;
}

const float& Camera::pitch()
{
	Camera& c = instance();
	return c.m_pitch;
}

const float& Camera::yaw()
{
	Camera& c = instance();
	return c.m_yaw;
}

const float& Camera::roll()
{
	Camera& c = instance();
	return c.m_roll;
}

void Camera::move(const float& dt, const MovementDirection direction)
{
	Camera& c = instance();
	//Update m_position vector
	switch (direction)
	{
	case MovementDirection::FORWARD:
		c.m_camera_position += c.m_camera_front * c.m_movement_speed * dt;
		break;
	case MovementDirection::BACKWARD:
		c.m_camera_position -= c.m_camera_front * c.m_movement_speed * dt;
		break;
	case MovementDirection::LEFT:
		c.m_camera_position -= c.m_camera_right * c.m_movement_speed * dt;
		break;
	case MovementDirection::RIGHT:
		c.m_camera_position += c.m_camera_right * c.m_movement_speed * dt;
		break;
	case MovementDirection::UP:
		c.m_camera_position += c.m_world_up * c.m_movement_speed * dt; // Elevates the camera in the specified world up direction
		break;
	case MovementDirection::DOWN:
		c.m_camera_position -= c.m_world_up * c.m_movement_speed * dt;
		break;
	default:
		break;
	}
	c.update(); // Compute the multiplications iff there is a change
}

void Camera::rotate(const float& dt, const double& offset_x, const double& offset_y)
{
	Camera& c = instance();
	//Update m_pitch and m_yaw
	c.m_pitch -= static_cast<float>(offset_y) * c.m_rotation_sensitivity * dt;
	c.m_yaw += static_cast<float>(offset_x) * c.m_rotation_sensitivity * dt;

	// FIX-ME-OPT: use proper quaternions to fix the gimball lock
	if (c.m_pitch > 89.999f && c.m_pitch <= 90.0f)
	{
		c.m_pitch = 90.001f;
	}
	else if (c.m_pitch < -89.999f && c.m_pitch >= -90.0f)
	{
		c.m_pitch = -90.001f;
	}
	if (c.m_pitch >= 360.0f)
	{
		c.m_pitch = 0.0f;
	}
	else if (c.m_pitch <= -360.0f)
	{
		c.m_pitch = 0.0f;
	}
	// Prevent yaw from accumulating
	if (c.m_yaw > 360.f || c.m_yaw < -360.f)
	{
		c.m_yaw = 0.f;
	}
	c.update(); // Compute the multiplications iff there is a change
}

void Camera::on_viewport_resize(int width, int height)
{
	Camera& c = instance();
	if (c.m_viewport_width != width 
		|| c.m_viewport_height != height)
	{
		c.m_viewport_width = width;
		c.m_viewport_height = height;
		// Only update the proj matrix
		c.m_proj_matrix = Angel::Perspective(c.m_fovy, c.m_viewport_width / (float)c.m_viewport_height, c.m_z_near, c.m_z_far);
	}
}

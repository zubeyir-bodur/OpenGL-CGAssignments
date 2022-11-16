#pragma once

#include "Angel-maths/mat.h"
#include "Angel-maths/quat.hpp"

/// <summary>
/// FIX-ME-OPT : Fix camera rotation
/// </summary>
class PerspectiveCamera
{
private:
	Angel::vec3 m_position = { 0.0f, 0.0f, 0.0f };
	Angel::quat m_rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
	float m_fovy = 60.0f;
	float m_aspect_ratio = 16.0f / 9.0f;
	Angel::mat4 m_proj_matrix = Angel::Perspective(m_fovy, m_aspect_ratio, 0.1f, 1e+38f); // Cache the proj matrix
	Angel::mat4 m_view_matrix = Angel::rotate(m_rotation) * Angel::Translate(m_position); // Cache the view matrix
	bool m_should_update = false;

	PerspectiveCamera() {};
	~PerspectiveCamera() {};
	PerspectiveCamera(const PerspectiveCamera&) = delete;
public:
	static PerspectiveCamera& get_instance();
	static void init(const Angel::vec3& position, float fovy, float aspect_ratio);
	static void pedestal(float dy);
	static void truck(float dx);
	static void dolly(float dz);
	static void pan(float d_thetax);
	static void tilt(float d_thetay);
	static void roll(float d_thetaz);
	static void fovy(float new_fovy);
	static void resize(float new_aspect_ratio);
	static const Angel::mat4& view_matrix();
	static const Angel::mat4& proj_matrix();
	static Angel::vec3& position();
	static Angel::quat& rotation();
	static void update();
};



class Camera
{
private:
	Angel::mat4 m_view_matrix = Angel::mat4(1.f);
	Angel::mat4 m_proj_matrix = Angel::mat4(1.f);

	float m_movement_speed = 750.0f;
	float m_rotation_sensitivity = 10.0f;
	float m_fovy;
	int m_viewport_width;
	int m_viewport_height;
	float m_z_near = 0.1f;
	float m_z_far = 1e+38f;

	Angel::vec3 m_world_up; // World up is the general up direction of the world, regardless from camera orientation
	Angel::vec3 m_camera_position;
	Angel::vec3 m_camera_front;		// Front direction relative to the camera rotation
	Angel::vec3 m_camera_right = {};	// Right direction relative to the camera rotation
	Angel::vec3 m_camera_up;			// Up direction relative to the camera rotation

	float m_pitch = 0.0f;
	float m_yaw = -90.0f;
	float m_roll = 0.0f;

	Camera() {}
	~Camera() {}
	Camera(const Camera& c) = delete;

	void update();

public:
	enum class MovementDirection { FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN };
	// Singleton utils
	static Camera& instance();
	static void init(Angel::vec3 position, Angel::vec3 up, float fov, int width, int height);
	//Accessors
	static const Angel::mat4& view_matrix();
	static const Angel::mat4& projection_matrix();
	static const Angel::vec3& position();
	static const float& pitch();
	static const float& yaw();
	static const float& roll();

	//Functions
	static void move(const float& dt, const MovementDirection direction);
	static void rotate(const float& dt, const double& offset_x, const double& offset_y);
	static void on_viewport_resize(int width, int height);

};
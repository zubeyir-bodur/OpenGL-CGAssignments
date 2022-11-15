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
	Angel::mat4 m_view_matrix = Angel::Rotate(m_rotation) * Angel::Translate(m_position); // Cache the view matrix
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
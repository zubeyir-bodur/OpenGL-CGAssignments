#pragma once
#include "Angel-maths/mat.h"

class OrthogtraphicCamera
{
private:
	Angel::vec3 m_camera_pos;
	float m_zoom_ratio;
	Angel::mat4 m_view_matrix;
	bool m_should_update = false;
	OrthogtraphicCamera() {};
	~OrthogtraphicCamera() {};
	OrthogtraphicCamera(const OrthogtraphicCamera&) = delete;
public:
	static OrthogtraphicCamera& get_instance();
	static void init(const Angel::vec3& pos, float zoom_ratio);
	static void move_vertical(float dy);
	static void move_horizontal(float dx);
	static void zoom_camera_towards(double zoom_percent_delta, double global_cursor_x, double global_cursor_y);
	static const Angel::vec3& position();
	static const Angel::mat4& view_matrix();
	static void update();
	static Angel::vec3 map_from_global(double x, double y);
	static Angel::vec3 map_to_global(const Angel::vec3& model);
	static const float& zoom_ratio();
};
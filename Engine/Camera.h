#pragma once
#include "Angel-maths/mat.h"

class Camera
{
private:
	Angel::vec3 m_camera_pos;
	float m_zoom_ratio;

	Camera();
	Camera(const Angel::vec3& pos, float zoom_ratio);
	~Camera();
	Camera(const Camera&) = delete;
	static Camera& get_instance();
public:
	static void init(const Angel::vec3& pos, float zoom_ratio);
	static void move_vertical(float dy);
	static void move_horizontal(float dx);
	static void zoom(double zoom_percent_delta, double global_cursor_x, double global_cursor_y);
	static const Angel::vec3 camera_pos();
	static Angel::mat4 view_matrix();
	static Angel::vec3 map_from_global(double x, double y);
	static const float& get_zoom_ratio();
};
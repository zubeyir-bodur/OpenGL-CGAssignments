#pragma once
#include "Angel-maths/mat.h"

class Camera2D
{
private:
	Angel::vec3 m_camera_pos;
	float m_zoom_ratio;
	Camera2D();
	~Camera2D();
	Camera2D(const Camera2D&) = delete;
public:
	static Camera2D& get_instance();
	static void init(const Angel::vec3& pos, float zoom_ratio);
	static void move_vertical(float dy);
	static void move_horizontal(float dx);
	static void zoom(double zoom_percent_delta, double global_cursor_x, double global_cursor_y);
	static const Angel::vec3 camera_pos();
	static Angel::mat4 view_matrix();
	static Angel::vec3 map_from_global(double x, double y);
	static Angel::vec3 map_to_global(const Angel::vec3& model);
	static const float& get_zoom_ratio();
};
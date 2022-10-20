#pragma once
#include "Shape.h"

class ShapeModel
{
public:
	enum class StaticShape
	{
		NONE,
		RECTANGLE,
		EQUILATERAL_TRIANGLE
	};
private:
	Shape* m_shape_def;
	StaticShape m_e_def;
	bool m_is_poly = false;
	bool m_is_selected = false;
	// Model dependent members
	Angel::vec3* m_position; // middle point of the geometric shape
	Angel::vec3* m_rotation; // in angles
	Angel::vec3* m_scale;	 // scaling from the middle point
	Angel::vec4* m_color;
public:
	// For predefined shapes
	ShapeModel(StaticShape def,
		Angel::vec3* pos,
		Angel::vec3* rot,
		Angel::vec3* scale,
		Angel::vec4* rgba);

	// For convex polygons
	ShapeModel(const std::vector<Angel::vec3>& poly_coords,
		Angel::vec3* pos,
		Angel::vec3* rot,
		Angel::vec3* scale,
		Angel::vec4* rgba);

	~ShapeModel();

	Angel::vec3& position() { return *m_position; }
	Angel::vec3& rotation() { return *m_rotation; }
	Angel::vec4& color() { return *m_color; }

	const VertexArray* vertex_array() { return m_shape_def->vertex_array(); }
	const IndexBuffer* triangles_index_buffer() { return m_shape_def->triangles_index_buffer(); }
	void select() { m_is_selected = true; }
	void deselect() { m_is_selected = false; }
	bool is_selected() { return m_is_selected; }
	StaticShape shape_def() { return m_e_def; }
	const float is_poly() { return m_is_poly; }
	bool contains(const Angel::vec3& model_pos);
	unsigned int true_num_vertices();
	std::vector<Angel::vec3> model_coords();
	Angel::mat4 model_matrix();
	void push_back_vertex(const Angel::vec3& model_pos);
	Angel::vec3 center_raw();
	std::array<float, 6> shape_bounding_cube();
	Angel::vec3 shape_size();
};
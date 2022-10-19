#pragma once
#include "Shape.h"

class ShapeModel
{
private:
	Shape* m_shape_def;
	bool m_is_poly = false;
	// Model dependent members
	Angel::vec3* m_position;
	Angel::vec3* m_rotation;
	Angel::vec3* m_scale;
	Angel::vec4* m_color;
public:
	enum class StaticShape
	{
		RECTANGLE,
		EQUILATERAL_TRIANGLE
	};

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

	const Angel::vec4& color() { return *m_color; }

	const VertexArray* vertex_array() { return m_shape_def->vertex_array(); }
	const IndexBuffer* index_buffer() { return m_shape_def->index_buffer(); }
	const float is_poly() { return m_is_poly; }
	bool contains(const Angel::vec3& model_pos);
	std::vector<Angel::vec3> model_coords();
	Angel::mat4 model_matrix();
	void push_back_vertex(const Angel::vec3& model_pos);
	Angel::vec3 center_position();
	std::array<float, 6> shape_bounding_cube();
	Angel::vec3 shape_size();
};
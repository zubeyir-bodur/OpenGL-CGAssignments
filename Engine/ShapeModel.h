#pragma once
#include "Shape.h"

class ShapeModel
{
private:
	Shape* m_shape_def;
	bool m_is_poly = false;
	// Model dependent members
	glm::vec3* m_position;
	glm::vec3* m_rotation;
	glm::vec3* m_scale;
	glm::vec4* m_color;
public:
	enum class StaticShape
	{
		RECTANGLE,
		EQUILATERAL_TRIANGLE
	};

	// For predefined shapes
	ShapeModel(StaticShape def, 
		glm::vec3* pos,
		glm::vec3* rot,
		glm::vec3* scale,
		glm::vec4* rgba);

	// For convex polygons
	ShapeModel(const std::vector<glm::vec3>& poly_coords,
		glm::vec3* pos,
		glm::vec3* rot,
		glm::vec3* scale,
		glm::vec4* rgba);

	~ShapeModel();

	const glm::vec4& color() { return *m_color; }

	const VertexArray* vertex_array() { return m_shape_def->vertex_array(); }
	const IndexBuffer* index_buffer() { return m_shape_def->index_buffer(); }
	const float is_poly() { return m_is_poly; }
	bool contains(const glm::vec3& model_pos);
	std::vector<glm::vec3> model_coords();
	glm::mat4 model_matrix();
	void push_back_vertex(const glm::vec3& model_pos);
	glm::vec3 center_position();
	std::array<float, 6> shape_bounding_cube();
	glm::vec3 shape_size();
};
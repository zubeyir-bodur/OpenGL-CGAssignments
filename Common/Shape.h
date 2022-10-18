#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
class Shape
{
public:
	glm::vec3* m_position; // position of the 0th vertex of the shape
	glm::vec3* m_rotation;
	glm::vec3* m_scale;
	float* m_color;
	std::vector<glm::vec3>* m_vertex_coordinates;
	VertexArray* m_vertex_array;
	IndexBuffer* m_index_buffer;
	VertexBufferLayout* m_layout;
	Shader* m_shader;

	// Polygon members
	bool is_poly = false;
	VertexBuffer* m_poly_vb = nullptr;
	unsigned int* m_poly_indices = nullptr;

	// Polygon constructor - basic shapes are statically defined
	Shape(const glm::vec3& pos,
		const glm::vec3& rot,
		const glm::vec3& scale,
		const float colour[4],
		const std::vector<glm::vec3>& coords,
		Shader* shader,
		VertexBufferLayout* basic_layout);
	~Shape();

	bool contains(const glm::vec3& model_pos);
	void push_back_vertex(const glm::vec3& model_pos);
	glm::vec3 center_position();
	std::array<float, 6> shape_bounding_cube();
	glm::vec3 shape_size();
};
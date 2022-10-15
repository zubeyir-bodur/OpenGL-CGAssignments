#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
struct Shape
{
	glm::vec3* m_position; // position of the 0th vertex of the shape
	glm::vec3* m_rotation;
	glm::vec3* m_scale;
	float* m_color;
	std::vector<glm::vec3>* m_vertex_coordinates;
	VertexArray* m_vertex_array;
	IndexBuffer* m_index_buffer;
	VertexBufferLayout* m_layout;
	Shader* m_shader;

	bool contains(float model_x, float model_y);
	void on_vertex_coordinates_changed(const std::vector<glm::vec3>& new_coords);
	glm::vec3 center_position();
	std::array<float, 6> shape_bounding_cube();
	glm::vec3 shape_size();
};
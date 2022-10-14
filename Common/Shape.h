#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>
struct Shape
{
	glm::vec3* m_position;
	glm::vec3* m_rotation;
	glm::vec3* m_scale;
	float* m_color;
	std::vector<glm::vec2>* m_vertex_coordinates;
	VertexArray* m_vertex_array;
	IndexBuffer* m_index_buffer;
	VertexBufferLayout* m_layout;
	Shader* m_shader;

	bool contains(float model_x, float model_y);
	void on_vertex_coordinates_changed(const std::vector<glm::vec2>& new_coords);
};
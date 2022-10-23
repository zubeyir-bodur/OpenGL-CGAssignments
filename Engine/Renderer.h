#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

struct GLFWwindow;

class Renderer
{
public:
	void draw_triangles(const VertexArray* vertex_array_obj, 
		const IndexBuffer* index_buffer_obj,
		const Shader* shader_obj) const;
	void draw_polygon(const VertexArray* vertex_array_obj,
			const IndexBuffer* index_buffer_obj,
			const Shader* shader_obj) const;
	void draw_lines(const VertexArray* vertex_array_obj,
		const IndexBuffer* index_buffer_obj,
		const Shader* shader_obj, int count = -1, const void* offset = nullptr) const;
	void clear(const float* clear_color) const;
	void set_viewport(GLFWwindow* window) const;

};
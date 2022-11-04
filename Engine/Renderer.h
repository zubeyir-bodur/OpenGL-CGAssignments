#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

struct GLFWwindow;

class Renderer
{
public:
	static void draw_triangles(const VertexArray* vertex_array_obj, 
		const IndexBuffer* index_buffer_obj,
		const Shader* shader_obj);
	static void draw_polygon(const VertexArray* vertex_array_obj,
			const IndexBuffer* index_buffer_obj,
			const Shader* shader_obj);
	static void draw_lines(const VertexArray* vertex_array_obj,
		const IndexBuffer* index_buffer_obj,
		const Shader* shader_obj, int count = -1, const void* offset = nullptr);
	static void clear(const float* clear_color);
	static void set_viewport(GLFWwindow* window);

};
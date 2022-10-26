#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Angel-maths/mat.h"
#include <vector>
#include <array>
#include <string>

#define NUM_COORDINATES 3

class Shape
{
private:
	// True shape defs that are constant for predefined shapes
	std::vector<float>* m_no_transform_vertex_positions;
	std::vector<unsigned int>* m_indices;
	VertexArray* m_vertex_array;
	VertexBuffer* m_vertex_buffer;
	IndexBuffer* m_triangles_index_buffer;

	// Static members
	static Shader* s_shader;
	static VertexBufferLayout s_layout;
	// Predefined shapes
	static Shape* s_eq_triangle;
	static Shape* s_rectangle;
public:
	Shape() {}
	// Convex polygon constructor
	Shape(const std::vector<Angel::vec3>& model_coords_center_translated_to_origin);
	~Shape();

	Angel::vec3 push_back_vertex(
		const Angel::vec3& new_vertex_pos_where_origin_is_old_center, 
		const Angel::vec3& old_center);
	unsigned int num_vertices();
	std::vector<float> vertices();
	const VertexArray* vertex_array() const				{ return m_vertex_array; }
	const IndexBuffer* triangles_index_buffer() const	{ return m_triangles_index_buffer; }

	static void init_static_members(int width);
	static void destroy_static_members_allocated_on_the_heap();
	static Shader* shader()						{ return s_shader; }
	static const VertexBufferLayout& layout()	{ return s_layout; }
	static const Shape* rectangle()				{ return s_rectangle; }
	static const Shape* eq_triangle()			{ return s_eq_triangle; }
};
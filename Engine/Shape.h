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
#define NUM_TEXTURE_COORDINATES 2
#define NUM_RGBA 4

class Shape
{
private:
	// True shape defs that are constant for predefined shapes
	std::vector<float>* m_no_transform_vertex_positions;
	std::vector<unsigned int>* m_indices;
	VertexArray* m_vertex_array;
	VertexBuffer* m_vertex_buffer;
	IndexBuffer* m_index_buffer;

	// Static members
	static Shader* s_basic_shader;
	static Shader* s_textured_shader;
	static Shader* s_colored_shader;
	static VertexBufferLayout* s_basic_layout;
	static VertexBufferLayout* s_textured_layout;
	static VertexBufferLayout* s_colored_layout;
	// Predefined shapes

	/// <summary>
	///         0    x
	///        / \   |
	///       /   \  H
	///      1--a--2 x
	///			 _  ___
	///           \/ 3
	///      H = -------, a = 1
	///             2
	///      
	/// </summary>
	static Shape* s_unit_eq_triangle;

	/// <summary>
	///  0----a---1   
	///  |        |   
	///  a        a   
	///  |        |
	///  3----a---2   
	///  
	///  a = 1  
	///
	/// </summary>
	static Shape* s_unit_square;

	/// <summary>
	/// Same orientation with colored & textured cubes
	/// 				19---a---16
	/// 				|         |
	/// 				a   Back  a
	///					|         |
	///					18---a---17
	///					 
	///				     0---------3
	///				    /   Top   /
	///				   1---------2
	///				   
	///                  7		  11
	///                 /|	      /|
	///                4 |	     8 |
	///                |L|	     |R|
	///                | 6	     | 10
	///                |/ 	     |/ 
	///                5		 9
	///               
	///                  20-------23
	///				    / Bottom /
	///				   21-------22
	///				
	///				   15---a---12
	///				   |         |
	/// 			   a  Front  a
	/// 			   |         |
	///				   14---a---13
	///  a = 1
	/// </summary>
	static Shape* s_colored_unit_cube;
	static Shape* s_textured_unit_cube;
public:
	Shape() {}
	// Convex polygon constructor
	Shape(const std::vector<Angel::vec3>& model_coords_center_translated_to_origin);
	~Shape();

	Angel::vec3 push_back_vertex(
		const Angel::vec3& new_vertex_pos_where_origin_is_old_center, 
		const Angel::vec3& old_center);

	std::vector<float> vertices();

	inline unsigned int num_vertices()							{ return (uint16_t)m_no_transform_vertex_positions->size() / NUM_COORDINATES; }
	inline const VertexArray* vertex_array() const				{ return m_vertex_array; }
	inline const IndexBuffer* index_buffer() const				{ return m_index_buffer; }

	static void init_static_members();
	static void destroy_static_members_allocated_on_the_heap();
	inline static Shader* basic_shader()						{ return s_basic_shader; }
	inline static Shader* textured_shader()						{ return s_textured_shader; }
	inline static Shader* colored_shader()						{ return s_colored_shader; }
	inline static const VertexBufferLayout& basic_layout()		{ return *s_basic_layout; }
	inline static const VertexBufferLayout& textured_layout()	{ return *s_textured_layout; }
	inline static const VertexBufferLayout& colored_layout()	{ return *s_colored_layout; }
	inline static const Shape* unit_square()					{ return s_unit_square; }
	inline static const Shape* unit_eq_triangle()				{ return s_unit_eq_triangle; }
	inline static const Shape* colored_unit_cube()				{ return s_colored_unit_cube; }
	inline static const Shape* textured_unit_cube()				{ return s_textured_unit_cube; }
};
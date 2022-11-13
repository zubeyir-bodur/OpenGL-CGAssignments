#include "Shape.h"
#include "ErrorManager.h"
#include <glew.h>

// Declare static members
Shader* Shape::s_basic_shader = nullptr;
Shader* Shape::s_textured_shader = nullptr;
VertexBufferLayout* Shape::s_basic_layout = nullptr;
VertexBufferLayout* Shape::s_textured_layout = nullptr;
Shape* Shape::s_unit_eq_triangle = new Shape;
Shape* Shape::s_unit_square = new Shape;
Shape* Shape::s_unit_cube = new Shape;

Shape::Shape(const std::vector<Angel::vec3>& model_coords_center_translated_to_origin)
{
	ASSERT(model_coords_center_translated_to_origin.size() >= 3);

	m_no_transform_vertex_positions = new std::vector<float>;
	m_no_transform_vertex_positions->reserve((model_coords_center_translated_to_origin.size() + 1) * NUM_COORDINATES);
	m_no_transform_vertex_positions->insert(m_no_transform_vertex_positions->begin(), 
		{
			0.0f,	0.0f,	0.0f
		});

	for (unsigned int i = NUM_COORDINATES/* 0*/; i < m_no_transform_vertex_positions->capacity(); i++)
	{
		m_no_transform_vertex_positions->emplace_back(model_coords_center_translated_to_origin[i / 3 - 1][i % 3]);
	}

	m_vertex_array = new VertexArray;
	m_vertex_buffer = new VertexBuffer(m_no_transform_vertex_positions->data(),
		m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, *s_basic_layout);
	
	m_indices = new std::vector<unsigned int>;
	m_indices->reserve(model_coords_center_translated_to_origin.size() + 2);
	for (unsigned int i = 0; i < model_coords_center_translated_to_origin.size() + 1; i++)
	{
		m_indices->emplace_back(i);
	}
	m_indices->emplace_back(1);
	m_index_buffer = new IndexBuffer(m_indices->data(), m_indices->size());
}

Shape::~Shape()
{
	delete m_vertex_array;
	delete m_vertex_buffer;
	delete m_index_buffer;
	delete m_indices;
	delete m_no_transform_vertex_positions;
}

/// <summary>
/// This function will be called whenever a vertex is added to the polygon
/// </summary>
/// <param name="centered_model_pos"></param>
Angel::vec3 Shape::push_back_vertex(const Angel::vec3& new_vertex_pos_where_origin_is_old_center, const Angel::vec3& old_center)
{
	ASSERT(this != s_unit_eq_triangle);
	ASSERT(this != s_unit_square);
	ASSERT(m_no_transform_vertex_positions->size() >= 3);
	delete m_vertex_buffer;
	delete m_vertex_array;
	delete m_index_buffer;
	m_no_transform_vertex_positions->insert(m_no_transform_vertex_positions->end(), 
		{
			new_vertex_pos_where_origin_is_old_center.x, new_vertex_pos_where_origin_is_old_center.y, new_vertex_pos_where_origin_is_old_center.z
		});

	// Update the center
	Angel::vec3 center(0.0f, 0.0f, 0.0f);
	for (unsigned int i = NUM_COORDINATES; i < num_vertices() * NUM_COORDINATES; i+=3)
	{
		center.x += (*m_no_transform_vertex_positions)[i];
		center.y += (*m_no_transform_vertex_positions)[i+1];
		center.z += (*m_no_transform_vertex_positions)[i+2];
	}
	center /= (float)num_vertices() - 1;

	// Move the vertices towards the new center
	for (unsigned int i = NUM_COORDINATES; i < num_vertices() * NUM_COORDINATES; i++)
	{
		(*m_no_transform_vertex_positions)[i] = ((*m_no_transform_vertex_positions)[i] - center[i % 3]);
	}

	// Recreate the vertex buffer & array
	m_vertex_array = new VertexArray;
	m_vertex_buffer = new VertexBuffer(m_no_transform_vertex_positions->data()
		, m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, *s_basic_layout);
	(*m_indices)[m_indices->size()-1] = num_vertices()-1;
	m_indices->emplace_back(1);
	m_index_buffer = new IndexBuffer(m_indices->data(), m_indices->size());

	// Return the new center as the position of the model
	return center + old_center;
}

std::vector<float> Shape::vertices()
{
	return std::vector<float>(*m_no_transform_vertex_positions);
}


void Shape::init_static_members()
{
	// Layout for basic shader
	s_basic_layout = new VertexBufferLayout();
	s_basic_layout->push_back_elements<float>(NUM_COORDINATES);

	// Basic shader
	s_basic_shader = new Shader("../../Engine/shaders/triangle.glsl");

	// Layout for textured shader
	s_textured_layout = new VertexBufferLayout();
	s_textured_layout->push_back_elements<float>(NUM_COORDINATES);
	s_textured_layout->push_back_elements<float>(NUM_TEXTURE_COORDINATES);

	// Textured Shader
	s_textured_shader = new Shader("../../Engine/shaders/textured_triangle.glsl");

	float unit = 1.0f;
	constexpr float global_z_pos_2d = 0.0f;

	// Index buffer for a quad composed of GL_TRIANGLES
	constexpr unsigned int num_indices = 6;
	auto* quad_indices = new std::vector<unsigned int>;
	quad_indices->reserve(num_indices);
	quad_indices->insert(quad_indices->end(),{ 
		0, 1, 2,
		2, 3, 0 
	});

	// Index buffer for a triangle
	auto* tri_indices = new std::vector<unsigned int>;
	tri_indices->reserve(num_indices/2);
	tri_indices->insert(tri_indices->end(), {
		0, 1, 2
	});

	// Index buffer for a cube composed of GL_QUADS
	constexpr unsigned int num_cube_vertices = 24;
	constexpr unsigned int num_cube_indices = 36;
	auto* cube_indices = new std::vector<unsigned int>;
	cube_indices->reserve(num_cube_indices);
	cube_indices->insert(cube_indices->end(), {
		// Left
		0,  1,  2,
		2,  3,	0,

		// Right
		4,  5,  6,
		6,  7,  4,

		// Top
		8,  9,  10,
		10, 11, 8,

		// Bottom
		12, 13, 14,
		14, 15, 12,

		// Front
		16, 17, 18,
		18, 19, 16,

		// Back
		20, 21, 22,
		22, 23, 20
		});

	// Create VAO, VBO & IBO for a rectangle
	constexpr unsigned int rect_num_vertices = 4;
	auto* rectangle_positions = new std::vector<float>;
	rectangle_positions->reserve(rect_num_vertices * NUM_COORDINATES);
	rectangle_positions->insert(rectangle_positions->begin(), {
		0.0f, 0.0f, global_z_pos_2d, // 0
		unit, 0.0f, global_z_pos_2d, // 1
		unit, unit, global_z_pos_2d, // 2
		0.0f, unit, global_z_pos_2d, // 3
	});

	auto* rect_va = new VertexArray;
	auto* rect_vb = new VertexBuffer(rectangle_positions->data(),
		rectangle_positions->size() * sizeof(float));
	rect_va->add_buffer(*rect_vb, *s_basic_layout);
	auto* rect_ib = new IndexBuffer(quad_indices->data(), num_indices);

	// Create VAO, VBO & IBO for an equilateral triangle
	constexpr unsigned int tri_num_vertices = 3;
	auto* equilateral_triangle_positions = new std::vector<float>;
	equilateral_triangle_positions->reserve(tri_num_vertices * NUM_COORDINATES);
	equilateral_triangle_positions->insert(equilateral_triangle_positions->begin(), {
		unit/2.0f,	0.0f,						global_z_pos_2d, // 0
		0.0f,		sqrtf(3)* unit / 2.0f,		global_z_pos_2d, // 1
		unit,		sqrtf(3)* unit / 2.0f,		global_z_pos_2d	 // 2
	});

	auto* eq_tri_va = new VertexArray;
	auto* eq_tri_vb = new VertexBuffer(equilateral_triangle_positions->data(),
		equilateral_triangle_positions->size() * sizeof(float));
	eq_tri_va->add_buffer(*eq_tri_vb, *s_basic_layout);
	auto* eq_tri_ib = new IndexBuffer(tri_indices->data(), num_indices / 2);

	// Create VAO, VBO & IBO for a
	auto* cube_positions = new std::vector<float>;
	cube_positions->reserve(num_cube_vertices * (NUM_COORDINATES + NUM_TEXTURE_COORDINATES));
	cube_positions->insert(cube_positions->begin(), {
		//	  X     Y    Z	    U     V
			// Left
			0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 0
			0.0f, 0.0f, unit, 0.0f, 1.0f, // 1
			0.0f, unit, unit, 0.0f, 0.0f, // 2
			0.0f, unit, 0.0f, 1.0f, 0.0f, // 3
			// Right		 		   
			unit, 0.0f, 0.0f, 0.0f, 1.0f, // 4
			unit, 0.0f, unit, 1.0f, 1.0f, // 5
			unit, unit, unit, 1.0f, 0.0f, // 6
			unit, unit, 0.0f, 0.0f, 0.0f, // 7
			// Top			 		   
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 8
			unit, 0.0f, 0.0f, 1.0f, 0.0f, // 9
			unit, 0.0f, unit, 1.0f, 1.0f, // 10
			0.0f, 0.0f, unit, 0.0f, 1.0f, // 11
			// Bottom		 		   
			0.0f, unit, 0.0f, 0.0f, 0.0f, // 12
			unit, unit, 0.0f, 1.0f, 0.0f, // 13
			unit, unit, unit, 1.0f, 1.0f, // 14
			0.0f, unit, unit, 0.0f, 1.0f, // 15
			// Front		 		   
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 16
			unit, 0.0f, 0.0f, 1.0f, 1.0f, // 17
			unit, unit, 0.0f, 1.0f, 0.0f, // 18
			0.0f, unit, 0.0f, 0.0f, 0.0f, // 19
			// Back			 		   
			0.0f, 0.0f, unit, 0.0f, 1.0f, // 20
			unit, 0.0f, unit, 1.0f, 1.0f, // 21
			unit, unit, unit, 1.0f, 0.0f, // 22
			0.0f, unit, unit, 0.0f, 0.0f, // 23
		});

	auto* cube_va = new VertexArray;
	auto* cube_vb = new VertexBuffer(cube_positions->data(),
		cube_positions->size() * sizeof(float));
	cube_va->add_buffer(*cube_vb, *s_basic_layout);
	auto* cube_ib = new IndexBuffer(cube_indices->data(), cube_indices->size());

	// Init static unit square
	s_unit_square->m_no_transform_vertex_positions = rectangle_positions;
	s_unit_square->m_indices = quad_indices;
	s_unit_square->m_vertex_array = rect_va;
	s_unit_square->m_vertex_buffer = rect_vb;
	s_unit_square->m_index_buffer = rect_ib;

	// Init static unit triangle
	s_unit_eq_triangle->m_no_transform_vertex_positions = equilateral_triangle_positions;
	s_unit_eq_triangle->m_indices = tri_indices;
	s_unit_eq_triangle->m_vertex_array = eq_tri_va;
	s_unit_eq_triangle->m_vertex_buffer = eq_tri_vb;
	s_unit_eq_triangle->m_index_buffer = eq_tri_ib;

	// Init static unit cube
	s_unit_cube->m_no_transform_vertex_positions = cube_positions;
	s_unit_cube->m_indices = cube_indices;
	s_unit_cube->m_vertex_array = cube_va;
	s_unit_cube->m_vertex_buffer = cube_vb;
	s_unit_cube->m_index_buffer = cube_ib;

	rect_va->unbind();
	rect_vb->unbind();
	rect_ib->unbind();
	eq_tri_va->unbind();
	eq_tri_vb->unbind();
	eq_tri_ib->unbind();
	cube_va->unbind();
	cube_vb->unbind();
	cube_ib->unbind();
	s_basic_shader->unbind();
	s_textured_shader->unbind();
}

void Shape::destroy_static_members_allocated_on_the_heap()
{
	delete s_unit_eq_triangle;
	delete s_unit_square;
	delete s_unit_cube;

	delete s_textured_layout;
	delete s_basic_layout;
	delete s_basic_shader;
	delete s_textured_shader;
}

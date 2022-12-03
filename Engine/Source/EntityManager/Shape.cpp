#include "EntityManager/Shape.h"
#include "Core/ErrorManager.h"
#include <glew.h>

// Declare static members
Shader* Shape::s_basic_shader = nullptr;
Shader* Shape::s_textured_shader = nullptr;
Shader* Shape::s_colored_shader = nullptr;
VertexBufferLayout* Shape::s_basic_layout = nullptr;
VertexBufferLayout* Shape::s_textured_layout = nullptr;
VertexBufferLayout* Shape::s_colored_layout = nullptr;
Shape* Shape::s_unit_eq_triangle = new Shape;
Shape* Shape::s_unit_square = new Shape;
Shape* Shape::s_colored_unit_cube = new Shape;
Shape* Shape::s_textured_unit_cube = new Shape;

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
		(uint16_t)m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, *s_basic_layout);
	
	m_indices = new std::vector<unsigned int>;
	m_indices->reserve(model_coords_center_translated_to_origin.size() + 2);
	for (unsigned int i = 0; i < model_coords_center_translated_to_origin.size() + 1; i++)
	{
		m_indices->emplace_back(i);
	}
	m_indices->emplace_back(1);
	m_index_buffer = new IndexBuffer(m_indices->data(), (uint16_t)m_indices->size());
}

Shape::~Shape()
{
	if (m_vertex_array)
	{
		delete m_vertex_array;
	}
	if (m_vertex_buffer)
	{
		delete m_vertex_buffer;
	}
	if (m_index_buffer)
	{
		delete m_index_buffer;
	}
	if (m_indices)
	{
		delete m_indices;
	}
	if (m_no_transform_vertex_positions)
	{
		delete m_no_transform_vertex_positions;
	}
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
		, (uint16_t)m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, *s_basic_layout);
	(*m_indices)[m_indices->size()-1] = num_vertices()-1;
	m_indices->emplace_back(1);
	m_index_buffer = new IndexBuffer(m_indices->data(), (uint16_t)m_indices->size());

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
	s_basic_shader = new Shader("../../Engine/Shaders/triangle.glsl");

	// Layout for textured and smooth shaded shader
	s_textured_layout = new VertexBufferLayout();
	s_textured_layout->push_back_elements<float>(NUM_COORDINATES);
	s_textured_layout->push_back_elements<float>(NUM_TEXTURE_COORDINATES);
	// Vertex normals for optional lighting - complete opaque object will still display the albedo color
	s_textured_layout->push_back_elements<float>(NUM_COORDINATES); 

	// Textured Shader
	s_textured_shader = new Shader("../../Engine/Shaders/textured_shaded_triangle.glsl");

	// Colored Shader
	s_colored_shader = new Shader("../../Engine/Shaders/colored_triangle.glsl");

	// Colored layout
	s_colored_layout = new VertexBufferLayout();
	s_colored_layout->push_back_elements<float>(NUM_COORDINATES);
	s_colored_layout->push_back_elements<float>(NUM_RGBA);

	float unit = 1.0f;
	float unit_half = 0.5f;
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
		// Top
		0, 1, 2,
		0, 2, 3,

		// Left
		5, 4, 6,
		6, 4, 7,

		// Right
		8, 9, 10,
		8, 10, 11,

		// Front
		13, 12, 14,
		15, 14, 12,

		// Back
		16, 17, 18,
		16, 18, 19,

		// Bottom
		21, 20, 22,
		22, 20, 23
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
		(uint16_t)rectangle_positions->size() * sizeof(float));
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
		(uint16_t)equilateral_triangle_positions->size() * sizeof(float));
	eq_tri_va->add_buffer(*eq_tri_vb, *s_basic_layout);
	auto* eq_tri_ib = new IndexBuffer(tri_indices->data(), num_indices / 2);

	// Create VAO, VBO & IBO for a colored cube
	auto* col_cube_positions = new std::vector<float>;
	col_cube_positions->reserve(num_cube_vertices * (NUM_COORDINATES + NUM_RGBA));

	col_cube_positions->insert(col_cube_positions->begin(), {
		//  X			Y			Z			 R		G		B    A
		// Top
		-unit_half,	unit_half,	-unit_half,		0.45f, 0.4f, 0.23f, 1.0f, // "Dirt" top for assignment 2
		-unit_half,	unit_half,	unit_half,		0.45f, 0.4f, 0.23f, 1.0f,
		unit_half,	unit_half,	unit_half,		0.45f, 0.4f, 0.23f, 1.0f,
		unit_half,	unit_half,	-unit_half,		0.45f, 0.4f, 0.23f, 1.0f,

		// Left
		-unit_half,	unit_half,	unit_half,		0.75f, 0.25f, 0.5f, 1.0f,
		-unit_half,	-unit_half,	unit_half,		0.75f, 0.25f, 0.5f, 1.0f,
		-unit_half,	-unit_half,	-unit_half,		0.75f, 0.25f, 0.5f, 1.0f,
		-unit_half,	unit_half,	-unit_half,		0.75f, 0.25f, 0.5f, 1.0f,

		// Right
		unit_half,	unit_half,	unit_half,		0.25f, 0.25f, 0.75f, 1.0f,
		unit_half,	-unit_half,	unit_half,		0.25f, 0.25f, 0.75f, 1.0f,
		unit_half,	-unit_half,	-unit_half,		0.25f, 0.25f, 0.75f, 1.0f,
		unit_half,	unit_half,	-unit_half,		0.25f, 0.25f, 0.75f, 1.0f,

		// Front
		unit_half,	unit_half,	unit_half,		1.0f, 0.0f, 0.15f,	1.0f,
		unit_half,	-unit_half,	unit_half,		1.0f, 0.0f, 0.15f,	1.0f,
		-unit_half,	-unit_half,	unit_half,		1.0f, 0.0f, 0.15f,	1.0f,
		-unit_half,	unit_half,	unit_half,		1.0f, 0.0f, 0.15f,	1.0f,

		// Back
		unit_half,	unit_half,	-unit_half,		0.0f, 1.0f, 0.15f,	1.0f,
		unit_half,	-unit_half,	-unit_half,		0.0f, 1.0f, 0.15f,	1.0f,
		-unit_half,	-unit_half,	-unit_half,		0.0f, 1.0f, 0.15f,	1.0f,
		-unit_half,	unit_half,	-unit_half,		0.0f, 1.0f, 0.15f,	1.0f,

		// Bottom
		-unit_half,	-unit_half,	-unit_half,		0.5f, 0.5f, 1.0f,	1.0f,
		-unit_half,	-unit_half,	unit_half,		0.5f, 0.5f, 1.0f,	1.0f,
		unit_half,	-unit_half,	unit_half,		0.5f, 0.5f, 1.0f,	1.0f,
		unit_half,	-unit_half,	-unit_half,		0.5f, 0.5f, 1.0f,	1.0f,
		});

	auto* col_cube_va = new VertexArray;
	auto* col_cube_vb = new VertexBuffer(col_cube_positions->data(),
		(uint16_t)col_cube_positions->size() * sizeof(float));
	col_cube_va->add_buffer(*col_cube_vb, *s_colored_layout);

	// Create VAO, VBO & IBO for a textured cube
	auto* tex_cube_positions = new std::vector<float>;
	tex_cube_positions->reserve(num_cube_vertices* (NUM_COORDINATES + NUM_TEXTURE_COORDINATES + NUM_COORDINATES));

	tex_cube_positions->insert(tex_cube_positions->begin(), {
		//  X			Y			Z			 U	  V			X_norm	Y_norm	Z_norm
		// Top 
		-unit_half,	unit_half,	-unit_half,		0.0f, 0.0f,		0.0f,	1.0f,	0.0f,
		-unit_half,	unit_half,	unit_half,		0.0f, 1.0f,		0.0f,	1.0f,	0.0f,
		unit_half,	unit_half,	unit_half,		1.0f, 1.0f,		0.0f,	1.0f,	0.0f,
		unit_half,	unit_half,	-unit_half,		1.0f, 0.0f,		0.0f,	1.0f,	0.0f,

		// Left
		-unit_half,	unit_half,	unit_half,		0.0f, 0.0f,		-1.0f,	0.0f,	0.0f,
		-unit_half,	-unit_half,	unit_half,		1.0f, 0.0f,		-1.0f,	0.0f,	0.0f,
		-unit_half,	-unit_half,	-unit_half,		1.0f, 1.0f,		-1.0f,	0.0f,	0.0f,
		-unit_half,	unit_half,	-unit_half,		0.0f, 1.0f,		-1.0f,	0.0f,	0.0f,
														  
		// Right							  
		unit_half,	unit_half,	unit_half,		1.0f, 1.0f,		1.0f,	0.0f,	0.0f,
		unit_half,	-unit_half,	unit_half,		0.0f, 1.0f,		1.0f,	0.0f,	0.0f,
		unit_half,	-unit_half,	-unit_half,		0.0f, 0.0f,		1.0f,	0.0f,	0.0f,
		unit_half,	unit_half,	-unit_half,		1.0f, 0.0f,		1.0f,	0.0f,	0.0f,

		// Front
		unit_half,	unit_half,	unit_half,		1.0f, 1.0f,		0.0f,	0.0f,	1.0f,
		unit_half,	-unit_half,	unit_half,		1.0f, 0.0f,		0.0f,	0.0f,	1.0f,
		-unit_half,	-unit_half,	unit_half,		0.0f, 0.0f,		0.0f,	0.0f,	1.0f,
		-unit_half,	unit_half,	unit_half,		0.0f, 1.0f,		0.0f,	0.0f,	1.0f,

		// Back
		unit_half,	unit_half,	-unit_half,		0.0f, 0.0f,		0.0f,	0.0f,	-1.0f,
		unit_half,	-unit_half,	-unit_half,		0.0f, 1.0f,		0.0f,	0.0f,	-1.0f,
		-unit_half,	-unit_half,	-unit_half,		1.0f, 1.0f,		0.0f,	0.0f,	-1.0f,
		-unit_half,	unit_half,	-unit_half,		1.0f, 0.0f,		0.0f,	0.0f,	-1.0f,

		// Bottom
		-unit_half,	-unit_half,	-unit_half,		1.0f, 1.0f,		0.0f,	-1.0f,	0.0f,
		-unit_half,	-unit_half,	unit_half,		1.0f, 0.0f,		0.0f,	-1.0f,	0.0f,
		unit_half,	-unit_half,	unit_half,		0.0f, 0.0f,		0.0f,	-1.0f,	0.0f,
		unit_half,	-unit_half,	-unit_half,		0.0f, 1.0f,		0.0f,	-1.0f,	0.0f,
		});

	auto* tex_cube_va = new VertexArray;
	auto* tex_cube_vb = new VertexBuffer(tex_cube_positions->data(),
		(uint16_t)tex_cube_positions->size() * sizeof(float));
	tex_cube_va->add_buffer(*tex_cube_vb, *s_textured_layout);
	
	// One IBO for both unit cubes
	auto* cube_ib = new IndexBuffer(cube_indices->data(), (uint16_t)cube_indices->size());

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

	// Init static colored unit cube
	s_colored_unit_cube->m_no_transform_vertex_positions = col_cube_positions;
	s_colored_unit_cube->m_vertex_array = col_cube_va;
	s_colored_unit_cube->m_vertex_buffer = col_cube_vb;
	
	// Init static textured unit cube
	s_textured_unit_cube->m_no_transform_vertex_positions = tex_cube_positions;
	s_textured_unit_cube->m_indices = cube_indices;
	s_textured_unit_cube->m_vertex_array = tex_cube_va;
	s_textured_unit_cube->m_vertex_buffer = tex_cube_vb;

	// Same IBO & indices for both cubes
	s_textured_unit_cube->m_indices = s_colored_unit_cube->m_indices = cube_indices;
	s_textured_unit_cube->m_index_buffer = s_colored_unit_cube->m_index_buffer = cube_ib;

	rect_va->unbind();
	rect_vb->unbind();
	rect_ib->unbind();
	eq_tri_va->unbind();
	eq_tri_vb->unbind();
	eq_tri_ib->unbind();
	col_cube_va->unbind();
	col_cube_vb->unbind();
	tex_cube_va->unbind();
	tex_cube_vb->unbind();
	cube_ib->unbind();
	s_basic_shader->unbind();
	s_textured_shader->unbind();
	s_colored_shader->unbind();
}

void Shape::destroy_static_members_allocated_on_the_heap()
{
	delete s_unit_eq_triangle;
	delete s_unit_square;
	delete s_colored_unit_cube;
	s_textured_unit_cube->m_indices = nullptr;
	s_textured_unit_cube->m_index_buffer = nullptr;
	delete s_textured_unit_cube;

	delete s_textured_layout;
	delete s_basic_layout;
	delete s_basic_shader;
	delete s_textured_shader;
}

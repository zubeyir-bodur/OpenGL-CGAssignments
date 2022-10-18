#include "Shape.h"
#include "ErrorManager.h"
#include <glew.h>

// Declare static members
Shader* Shape::s_shader = nullptr;
VertexBufferLayout Shape::s_layout;
Shape* Shape::s_eq_triangle = new Shape();
Shape* Shape::s_rectangle = new Shape();

Shape::Shape(const std::vector<glm::vec3>& coords)
{
	m_no_transform_vertex_positions = new std::vector<float>(coords.size() * NUM_COORDINATES);
	for (unsigned int i = 0; i < m_no_transform_vertex_positions->size(); i++)
	{
		(*m_no_transform_vertex_positions)[i] = coords[i / 3][i % 3];
	}

	m_vertex_array = new VertexArray;
	m_vertex_buffer = new VertexBuffer(m_no_transform_vertex_positions->data()
		, m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, s_layout);
	m_indices = new std::vector<unsigned int>(coords.size());
	for (unsigned int i = 0; i < coords.size(); i++)
	{
		(*m_indices)[i] = i;
	}
	m_index_buffer = new IndexBuffer(m_indices->data(), coords.size());
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
/// <param name="model_pos"></param>
void Shape::push_back_vertex(const glm::vec3& model_pos)
{
	ASSERT(this != s_eq_triangle);
	ASSERT(this != s_rectangle);
	delete m_vertex_array;
	delete m_vertex_buffer;
	delete m_indices;
	delete m_index_buffer;
	m_no_transform_vertex_positions->push_back(model_pos.x);
	m_no_transform_vertex_positions->push_back(model_pos.y);
	m_no_transform_vertex_positions->push_back(model_pos.z);
	m_vertex_array = new VertexArray;
	m_vertex_buffer = new VertexBuffer(m_no_transform_vertex_positions->data(), 
		m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, s_layout);
	m_indices = new std::vector<unsigned int>(m_no_transform_vertex_positions->size() / NUM_COORDINATES);
	for (unsigned int i = 0; i < m_indices->size(); i++)
	{
		(*m_indices)[i] = i;
	}
	m_index_buffer = new IndexBuffer(m_indices->data(), m_indices->size());
}

unsigned int Shape::num_vertices()
{
	return m_no_transform_vertex_positions->size() / NUM_COORDINATES;
}

const std::vector<float>& Shape::vertices()
{
	return *m_no_transform_vertex_positions;
}


void Shape::init_static_members(int width)
{
	// Single color, texture-less, 3D vertex buffer layout
	s_layout = VertexBufferLayout();
	s_layout.push_back_elements<float>(NUM_COORDINATES);

	s_shader = new Shader("../../Engine/shaders/triangle.glsl");

	float init_shape_length = width / 8.0f;
	constexpr float global_z_pos_2d = 0.0f;

	// Index buffer for a 2D quad
	constexpr unsigned int num_indices = 6;
	auto* quad_indices = new std::vector<unsigned int>{
		0, 1, 2,
		2, 3, 0
	};
	auto* tri_indices = new std::vector<unsigned int>{
		0, 1, 2
	};

	// Create the vertex buffer for a rectangle
	constexpr unsigned int rect_num_vertices = 4;
	auto* rectangle_positions = new std::vector<float>{
		0.0f,				0.0f,				global_z_pos_2d,	// 0
		init_shape_length,	0.0f,				global_z_pos_2d,	// 1
		init_shape_length,	init_shape_length,	global_z_pos_2d,	// 2
		0.0f,				init_shape_length,	global_z_pos_2d		// 3
	};
	auto* rect_va = new VertexArray;
	auto* rect_vb = new VertexBuffer(rectangle_positions->data(),
		rectangle_positions->size() * sizeof(float));
	rect_va->add_buffer(*rect_vb, s_layout);
	auto* rect_ib = new IndexBuffer(quad_indices->data(), num_indices);

	// Vertex Buffer for an equilateral triangle
	constexpr unsigned int tri_num_vertices = 3;
	auto* equilateral_triangle_positions = new std::vector<float>{
		init_shape_length / 2,	0.0f,								global_z_pos_2d,	// 0
		0.0f,					sqrtf(3) * init_shape_length / 2,	global_z_pos_2d,	// 1
		init_shape_length,		sqrtf(3) * init_shape_length / 2,	global_z_pos_2d,	// 2
	};
	auto* eq_tri_va = new VertexArray;
	auto* eq_tri_vb = new VertexBuffer(equilateral_triangle_positions->data(),
		equilateral_triangle_positions->size() * sizeof(float));
	eq_tri_va->add_buffer(*eq_tri_vb, s_layout);
	auto* eq_tri_ib = new IndexBuffer(tri_indices->data(), num_indices / 2);

	// Init static rectangle
	s_rectangle->m_no_transform_vertex_positions = rectangle_positions;
	s_rectangle->m_indices = quad_indices;
	s_rectangle->m_vertex_array = rect_va;
	s_rectangle->m_vertex_buffer = rect_vb;
	s_rectangle->m_index_buffer = rect_ib;

	s_eq_triangle->m_no_transform_vertex_positions = equilateral_triangle_positions;
	s_eq_triangle->m_indices = tri_indices;
	s_eq_triangle->m_vertex_array = eq_tri_va;
	s_eq_triangle->m_vertex_buffer = eq_tri_vb;
	s_eq_triangle->m_index_buffer = eq_tri_ib;

	rect_va->unbind();
	rect_vb->unbind();
	rect_ib->unbind();
	eq_tri_va->unbind();
	eq_tri_vb->unbind();
	eq_tri_ib->unbind();
	s_shader->unbind();
}

void Shape::destroy_static_members_allocated_on_the_heap()
{
	delete s_eq_triangle;
	delete s_rectangle;
}

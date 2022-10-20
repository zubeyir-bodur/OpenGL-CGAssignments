#include "Shape.h"
#include "ErrorManager.h"
#include <glew.h>

// Declare static members
Shader* Shape::s_shader = nullptr;
VertexBufferLayout Shape::s_layout;
Shape* Shape::s_eq_triangle = new Shape();
Shape* Shape::s_rectangle = new Shape();

Shape::Shape(const std::vector<Angel::vec3>& coords)
{
	ASSERT(coords.size() >= 3);
	m_no_transform_vertex_positions = new std::vector<float>;
	m_no_transform_vertex_positions->reserve((coords.size() + 1) * NUM_COORDINATES);
	Angel::vec3 center(0.0f, 0.0f, 0.0f);
	for (auto& coord : coords)
	{
		center.x += coord.x;
		center.y += coord.y;
		center.z += coord.z;
	}
	center /= (float)coords.size();
	m_no_transform_vertex_positions->emplace_back(center.x);
	m_no_transform_vertex_positions->emplace_back(center.y);
	m_no_transform_vertex_positions->emplace_back(center.z);
	for (unsigned int i = NUM_COORDINATES; i < m_no_transform_vertex_positions->capacity(); i++)
	{
		m_no_transform_vertex_positions->emplace_back(coords[i / 3 - 1][i % 3]);
	}

	m_vertex_array = new VertexArray;
	m_vertex_buffer = new VertexBuffer(m_no_transform_vertex_positions->data()
		, m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, s_layout);
	
	m_indices = new std::vector<unsigned int>;
	m_indices->reserve(coords.size() + 2);
	for (unsigned int i = 0; i < coords.size() + 1; i++)
	{
		m_indices->emplace_back(i);
	}
	m_indices->emplace_back(1);
	m_triangles_index_buffer = new IndexBuffer(m_indices->data(), m_indices->size());
}

Shape::~Shape()
{
	delete m_vertex_array;
	delete m_vertex_buffer;
	delete m_triangles_index_buffer;
	delete m_indices;
	delete m_no_transform_vertex_positions;
}

/// <summary>
/// This function will be called whenever a vertex is added to the polygon
/// </summary>
/// <param name="model_pos"></param>
void Shape::push_back_vertex(const Angel::vec3& model_pos)
{
	ASSERT(this != s_eq_triangle);
	ASSERT(this != s_rectangle);
	ASSERT(m_no_transform_vertex_positions->size() >= 3);
	delete m_vertex_buffer;
	delete m_vertex_array;
	delete m_triangles_index_buffer;
	m_no_transform_vertex_positions->emplace_back(model_pos.x);
	m_no_transform_vertex_positions->emplace_back(model_pos.y);
	m_no_transform_vertex_positions->emplace_back(model_pos.z);
	Angel::vec3 center(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < num_vertices(); i+=3)
	{
		center.x += (*m_no_transform_vertex_positions)[i];
		center.y += (*m_no_transform_vertex_positions)[i+1];
		center.z += (*m_no_transform_vertex_positions)[i+2];
	}
	center /= (float)num_vertices();

	// Update the center
	(*m_no_transform_vertex_positions)[0] = center.x;
	(*m_no_transform_vertex_positions)[1] = center.y;
	(*m_no_transform_vertex_positions)[2] = center.z;

	// Recreate the vertex buffer & array
	m_vertex_array = new VertexArray;
	m_vertex_buffer = new VertexBuffer(m_no_transform_vertex_positions->data()
		, m_no_transform_vertex_positions->size() * sizeof(float));
	m_vertex_array->add_buffer(*m_vertex_buffer, s_layout);
	(*m_indices)[m_indices->size()-1] = num_vertices()-1;
	m_indices->emplace_back(1);
	m_triangles_index_buffer = new IndexBuffer(m_indices->data(), m_indices->size());
}

unsigned int Shape::num_vertices()
{
	return m_no_transform_vertex_positions->size() / NUM_COORDINATES;
}

std::vector<float> Shape::vertices()
{
	return std::vector<float>(*m_no_transform_vertex_positions);
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
	auto* quad_indices = new std::vector<unsigned int>;
	quad_indices->reserve(num_indices);
	quad_indices->emplace_back(0); quad_indices->emplace_back(1); quad_indices->emplace_back(2);
	quad_indices->emplace_back(2); quad_indices->emplace_back(3); quad_indices->emplace_back(0);

	auto* tri_indices = new std::vector<unsigned int>;
	tri_indices->reserve(num_indices/2);
	tri_indices->emplace_back(0); tri_indices->emplace_back(1); tri_indices->emplace_back(2);

	// Create the vertex buffer for a rectangle
	constexpr unsigned int rect_num_vertices = 4;
	auto* rectangle_positions = new std::vector<float>;
	rectangle_positions->reserve(rect_num_vertices * NUM_COORDINATES);
	rectangle_positions->emplace_back(0.0f);				rectangle_positions->emplace_back(0.0f);				rectangle_positions->emplace_back(global_z_pos_2d); // 0
	rectangle_positions->emplace_back(init_shape_length);	rectangle_positions->emplace_back(0.0f);				rectangle_positions->emplace_back(global_z_pos_2d); // 1
	rectangle_positions->emplace_back(init_shape_length);	rectangle_positions->emplace_back(init_shape_length);	rectangle_positions->emplace_back(global_z_pos_2d); // 2
	rectangle_positions->emplace_back(0.0f);				rectangle_positions->emplace_back(init_shape_length);	rectangle_positions->emplace_back(global_z_pos_2d); // 3

	auto* rect_va = new VertexArray;
	auto* rect_vb = new VertexBuffer(rectangle_positions->data(),
		rectangle_positions->size() * sizeof(float));
	rect_va->add_buffer(*rect_vb, s_layout);
	auto* rect_ib = new IndexBuffer(quad_indices->data(), num_indices);

	// Vertex Buffer for an equilateral triangle
	constexpr unsigned int tri_num_vertices = 3;
	auto* equilateral_triangle_positions = new std::vector<float>;
	equilateral_triangle_positions->reserve(tri_num_vertices * NUM_COORDINATES);
	equilateral_triangle_positions->emplace_back(init_shape_length / 2);	equilateral_triangle_positions->emplace_back(0.0f);								equilateral_triangle_positions->emplace_back(global_z_pos_2d); // 0
	equilateral_triangle_positions->emplace_back(0.0f);						equilateral_triangle_positions->emplace_back(sqrtf(3) * init_shape_length / 2);	equilateral_triangle_positions->emplace_back(global_z_pos_2d); // 1
	equilateral_triangle_positions->emplace_back(init_shape_length);		equilateral_triangle_positions->emplace_back(sqrtf(3) * init_shape_length / 2);	equilateral_triangle_positions->emplace_back(global_z_pos_2d); // 2

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
	s_rectangle->m_triangles_index_buffer = rect_ib;

	s_eq_triangle->m_no_transform_vertex_positions = equilateral_triangle_positions;
	s_eq_triangle->m_indices = tri_indices;
	s_eq_triangle->m_vertex_array = eq_tri_va;
	s_eq_triangle->m_vertex_buffer = eq_tri_vb;
	s_eq_triangle->m_triangles_index_buffer = eq_tri_ib;

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

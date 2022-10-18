#include "Shape.h"
#include "ErrorManager.h"
#include <glew.h>

Shape::Shape(const glm::vec3& pos,
	const glm::vec3& rot,
	const glm::vec3& scale,
	const float colour[4],
	const std::vector<glm::vec3>& coords,
	Shader* shader,
	VertexBufferLayout* basic_layout
	)
{
	m_shader = shader;
	m_layout = basic_layout;

	is_poly = true;
	m_position = new glm::vec3(pos);
	m_rotation = new glm::vec3(rot);
	m_scale = new glm::vec3(scale);
	
	m_color = new float[4];
	for (unsigned int i = 0; i < 4; i++)
	{
		m_color[i] = colour[i];
	}
	
	m_vertex_coordinates = new std::vector<glm::vec3>(coords.size());
	for (unsigned int i = 0; i < coords.size(); i++)
	{
		m_vertex_coordinates->push_back(coords[i]);
	}

	m_vertex_array = new VertexArray;
	m_poly_vb = new VertexBuffer(&coords.data()[0], coords.size() * 3 * sizeof(float));
	m_vertex_array->add_buffer(*m_poly_vb, *basic_layout);
	m_poly_indices = new unsigned int[coords.size()];
	for (unsigned int i = 0; i < coords.size(); i++)
	{
		m_poly_indices[i] = i;
	}
	m_index_buffer = new IndexBuffer(m_poly_indices, coords.size());
}

Shape::~Shape()
{
	if (is_poly)
	{
		delete m_position;
		delete m_rotation;
		delete m_scale;
		delete m_color;
		delete m_vertex_coordinates;
		delete m_vertex_array;
		delete m_poly_vb;
		delete m_poly_indices;
		delete m_index_buffer;
	}
}

bool Shape::contains(const glm::vec3& model_pos)
{
	unsigned int nvert = m_vertex_coordinates->size() * 3;
	unsigned int i, j;
	bool c = false;
	ASSERT(!m_layout->elements().empty() &&
		m_layout->elements()[0].count == 3 &&
		m_layout->elements()[0].type == GL_FLOAT);
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (
			((m_vertex_coordinates[0][i].y > model_pos.y) != (m_vertex_coordinates[0][j].y > model_pos.y)) &&
			(model_pos.x < (m_vertex_coordinates[0][j].x - m_vertex_coordinates[0][i].x) * (model_pos.y - m_vertex_coordinates[0][i].y) / (m_vertex_coordinates[0][j].y - m_vertex_coordinates[0][i].y) + m_vertex_coordinates[0][j].x)
			)
			c = !c;
	}
	return c;
}

/// <summary>
/// This function will be called whenever a vertex is added to the polygon
/// </summary>
/// <param name="model_pos"></param>
void Shape::push_back_vertex(const glm::vec3& model_pos)
{
	ASSERT(is_poly);
	delete m_vertex_array;
	delete m_poly_vb;
	delete m_poly_indices;
	delete m_index_buffer;
	m_vertex_coordinates->push_back(model_pos);
	m_vertex_array = new VertexArray;
	m_poly_vb = new VertexBuffer(&m_vertex_coordinates->data()[0], m_vertex_coordinates->size() * 3 * sizeof(float));
	m_vertex_array->add_buffer(*m_poly_vb, *m_layout);
	m_poly_indices = new unsigned int[m_vertex_coordinates->size()];
	for (unsigned int i = 0; i < m_vertex_coordinates->size(); i++)
	{
		m_poly_indices[i] = i;
	}
	m_index_buffer = new IndexBuffer(m_poly_indices, m_vertex_coordinates->size());
}

/// <summary>
/// Position of the center of the shape in terms of model coordinates
/// </summary>
/// <returns></returns>
glm::vec3 Shape::center_position()
{
	glm::vec3 center(0.0f, 0.0f, 0.0f);
	for (const auto& vertex : *m_vertex_coordinates)
	{
		center.x += (vertex.x - m_position->x) * m_scale->x + 2 * m_position->x;
		center.y += (vertex.y - m_position->y) * m_scale->y + 2 * m_position->y;
		center.z += (vertex.z - m_position->z) * m_scale->z + 2 * m_position->z;
	}
	center /= (float)m_vertex_coordinates->size();
	return center;
}

std::array<float, 6> Shape::shape_bounding_cube()
{
	float x_min = (float)INT_MAX, x_max = (float)INT_MIN, y_min = (float)INT_MAX, y_max = (float)INT_MIN, z_min = (float)INT_MAX, z_max = (float)INT_MIN;
	if (m_vertex_coordinates->empty())
	{
		return { 0, 0, 0, 0, 0, 0 };
	}
	for (auto& vert_coord : *m_vertex_coordinates)
	{
		if (vert_coord.x < x_min)
		{
			x_min = vert_coord.x;
		}
		if (vert_coord.x > x_max)
		{
			x_max = vert_coord.x;
		}
		if (vert_coord.y < y_min)
		{
			y_min = vert_coord.y;
		}
		if (vert_coord.y > y_max)
		{
			y_max = vert_coord.y;
		}
		if (vert_coord.z < z_min)
		{
			z_min = vert_coord.z;
		}
		if (vert_coord.z > z_max)
		{
			z_max = vert_coord.z;
		}
	}
	return {
		x_min, x_max, y_min, y_max, z_min, z_max
	};
}

/// <summary>
/// 2D sizes of the shape in terms of model coordinates
/// </summary>
/// <returns></returns>
glm::vec3 Shape::shape_size()
{
	std::array<float, 6> bounding_cube = this->shape_bounding_cube();
	return {
		(bounding_cube[1] - bounding_cube[0]) * m_scale->x,
		(bounding_cube[3] - bounding_cube[2]) * m_scale->y,
		(bounding_cube[5] - bounding_cube[4]) * m_scale->z
	};
}

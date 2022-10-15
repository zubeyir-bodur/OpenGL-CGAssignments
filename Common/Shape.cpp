#include "Shape.h"
#include "ErrorManager.h"
#include <glew.h>

bool Shape::contains(float model_x, float model_y)
{
	unsigned int nvert = m_vertex_coordinates->size() * 2;
	unsigned int i, j, c = 0;
	ASSERT(!m_layout->elements().empty() &&
		m_layout->elements()[0].count == 2 &&
		m_layout->elements()[0].type == GL_FLOAT);
	// Assuming the data coordinates are initial model coordinates
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (
			((m_vertex_coordinates[0][i].y > model_y) != (m_vertex_coordinates[0][j].y > model_y)) &&
			(model_x < (m_vertex_coordinates[0][j].x - m_vertex_coordinates[0][i].x) * (model_y - m_vertex_coordinates[0][i].y) / (m_vertex_coordinates[0][j].y - m_vertex_coordinates[0][i].y) + m_vertex_coordinates[0][j].x)
			)
			c = !c;
	}
	return (bool)c;
}

/// <summary>
/// This function will be called if the outline of the shape 
/// is changed other than transformations, such as adding a vertex
/// resizing from the edges (not from the corner) or other things that
/// require the all the buffers to be rebound
/// </summary>
/// <param name="r"></param>
/// <param name="proj"></param>
/// <param name="view"></param>
void Shape::on_vertex_coordinates_changed(const std::vector<glm::vec3>& new_coords)
{
	// Update the vertex buffer, vertex array and layout if necessary
	// TODO, necessary for displaying polygons as they are drawn
	return;
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

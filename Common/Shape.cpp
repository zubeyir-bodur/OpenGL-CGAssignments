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
void Shape::on_vertex_coordinates_changed(const std::vector<glm::vec2>& new_coords)
{
	// Update the vertex buffer, vertex array and layout if necessary
	// TODO, necessary for displaying polygons as they are drawn
	return;
}
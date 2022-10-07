#include "DrawList.h"
#include "ErrorManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glew.h"

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

DrawList::DrawList(Renderer* r, const glm::mat4& proj, const glm::mat4& view)
{
	m_renderer = r;
	m_proj_mat = const_cast<glm::mat4*>(&proj);
	m_view_mat = const_cast<glm::mat4*>(&view);
}
DrawList::~DrawList()
{
	m_renderer = nullptr;
}

void DrawList::add_shape(Shape* s)
{
	m_shapes.push_back(s);
}

void DrawList::move_shape_to_frontview(Shape* s)
{
	unsigned int idx = idx_of(s);
	assert(idx != -1);
	m_shapes.erase(m_shapes.begin() + idx);
	m_shapes.push_back(s);
}

Shape* DrawList::frontmost_shape_at_pos(float model_x, float model_y)
{
	for (int i = m_shapes.size() - 1; i >= 0; i--)
	{
		if (m_shapes[i]->contains(model_x, model_y))
		{
			return m_shapes[i];
		}
	}
	return nullptr;
}

unsigned int DrawList::idx_of(Shape* s)
{
	for (unsigned int i = 0; i < m_shapes.size(); i++)
	{
		if (m_shapes[i] == s)
		{
			return i;
		}
	}
	return -1;
}

void DrawList::draw_all()
{
	for (auto shape : m_shapes)
	{
		glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), *shape->m_model_positions);
		glm::mat4 MVP_matrix = (* m_proj_mat) * (*m_view_mat) * model_matrix;

		// Update locations and colors
		shape->m_shader->bind();
		shape->m_shader->set_uniform_4f("u_color",
			shape->m_model_colors[0],
			shape->m_model_colors[1],
			shape->m_model_colors[2],
			shape->m_model_colors[3]);
		shape->m_shader->set_uniform_mat4f("u_MVP", MVP_matrix);
		m_renderer->draw(shape->m_vertex_array, shape->m_index_buffer, shape->m_shader);
	}
}


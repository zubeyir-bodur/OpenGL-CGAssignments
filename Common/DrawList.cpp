#include "DrawList.h"
#include "ErrorManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glew.h>

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
		glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), *shape->m_position) 
			* glm::rotate(glm::mat4(1.0f), (*shape->m_rotation).x, glm::vec3(1, 0, 0))
			* glm::rotate(glm::mat4(1.0f), (*shape->m_rotation).y, glm::vec3(0, 1, 0))
			* glm::rotate(glm::mat4(1.0f), (*shape->m_rotation).z, glm::vec3(0, 0, 1))
			* glm::scale(glm::mat4(1.0f), (*shape->m_scale));
		glm::mat4 MVP_matrix = (* m_proj_mat) * (*m_view_mat) * model_matrix;

		// Update locations and colors
		shape->m_shader->bind();
		shape->m_shader->set_uniform_4f("u_color",
			shape->m_color[0],
			shape->m_color[1],
			shape->m_color[2],
			shape->m_color[3]);
		shape->m_shader->set_uniform_mat4f("u_MVP", MVP_matrix);
		m_renderer->draw(shape->m_vertex_array, shape->m_index_buffer, shape->m_shader);
	}
}


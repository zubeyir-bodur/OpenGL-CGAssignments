#include "DrawList.h"
#include "ErrorManager.h"
#include "Angel-maths/mat.h"
#include <glew.h>

DrawList::DrawList(Renderer* r, const Angel::mat4& proj, const Angel::mat4& view)
{
	m_renderer = r;
	m_proj_mat = const_cast<Angel::mat4*>(&proj);
	m_view_mat = const_cast<Angel::mat4*>(&view);
}
DrawList::~DrawList()
{
	m_renderer = nullptr;
}

void DrawList::add_shape(ShapeModel* s)
{
	m_shape_models.push_back(s);
}

void DrawList::move_shape_to_frontview(ShapeModel* s)
{
	unsigned int idx = idx_of(s);
	ASSERT(idx != -1);
	m_shape_models.erase(m_shape_models.begin() + idx);
	m_shape_models.push_back(s);
}

ShapeModel* DrawList::frontmost_shape(const Angel::vec3& model_pos)
{
	for (int i = m_shape_models.size() - 1; i >= 0; i--)
	{
		if (m_shape_models[i]->contains(model_pos))
		{
			return m_shape_models[i];
		}
	}
	return nullptr;
}

unsigned int DrawList::idx_of(ShapeModel* s)
{
	for (unsigned int i = 0; i < m_shape_models.size(); i++)
	{
		if (m_shape_models[i] == s)
		{
			return i;
		}
	}
	return -1;
}

/// <summary>
/// Currently, scaling is not supported together with rotation
/// </summary>
void DrawList::draw_all()
{
	for (auto shape : m_shape_models)
	{
		Angel::mat4 model_mat = shape->model_matrix();
		std::vector<Angel::vec3> coords = shape->model_coords();
		Angel::mat4 MVP_matrix = (* m_proj_mat) * (*m_view_mat) * model_mat;

		// Update locations and colors
		Shape::shader()->bind();
		Shape::shader()->set_uniform_4f("u_color",
			shape->color()[0],
			shape->color()[1],
			shape->color()[2],
			shape->color()[3]);
		Shape::shader()->set_uniform_mat4f("u_MVP", MVP_matrix);

		// draw
		if (!shape->is_poly())
		{
			m_renderer->draw_triangles(shape->vertex_array(), shape->index_buffer(), Shape::shader());
			// TODO
			// if(shaoe->is_selected())
			// {
			//		Shape::shader()->set_uniform_4f("u_color",
			//			0.0f,
			//			0.0f,
			//			0.0f,
			//			1.0f);
			//		m_renderer->draw_lines(shape->vertex_array(), shape->index_buffer(), Shape::shader());
			//	}
		}
		else
		{
			m_renderer->draw_polygon(shape->vertex_array(), shape->index_buffer(), Shape::shader());
		}
	}
}


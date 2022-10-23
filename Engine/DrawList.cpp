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
	//for (auto& ptr : m_shape_models)
	//{
	//	delete ptr;
	//}
	m_shape_models.clear();
	// TODO avoid mem-leaks
}

void DrawList::add_shape(ShapeModel* s)
{
	m_shape_models.push_back(s);
}

void DrawList::remove_shape(ShapeModel* s)
{
	unsigned int idx = idx_of(s);
	m_shape_models.erase(m_shape_models.begin() + idx);
	if (s != nullptr)
	{
		delete s;
	}
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

const std::vector<ShapeModel*> DrawList::shapes_contained_in(const Angel::vec3& selector_pos, 
	const Angel::vec3& selector_scale)
{
	std::vector<ShapeModel*> out;
	out.reserve(m_shape_models.size());
	auto* tmp_pos = new Angel::vec3(selector_pos);
	auto* tmp_scale = new Angel::vec3(selector_scale);
	auto* tmp_rot = new Angel::vec3(0, 0, 0);
	auto* tmp_col = new Angel::vec4(0, 0, 0, 0);
	ShapeModel selection_rectangle_sm = ShapeModel(ShapeModel::StaticShape::RECTANGLE, tmp_pos, tmp_rot, tmp_scale, tmp_col);
	for (unsigned int i = 0; i < m_shape_models.size(); i++)
	{
		bool in = false;
		for (unsigned int j = 0; j < m_shape_models[i]->true_num_vertices(); j++)
		{
			Angel::vec3 point_j = m_shape_models[i]->model_coords()[j];
			// If at least one vertex is inside the region - excluding the borders of this region
			// Then the shape is inside this region
			if (selection_rectangle_sm.contains(point_j))
			{
				out.emplace_back(m_shape_models[i]);
				in = true;
				break;
			}
		}
		if (!in)
		{
			// Or, the shape might contain the selection rectangle
			for (unsigned int j = 0; j < selection_rectangle_sm.true_num_vertices(); j++)
			{
				Angel::vec3 point_j = selection_rectangle_sm.model_coords()[j];
				if (m_shape_models[i]->contains(point_j))
				{
					out.emplace_back(m_shape_models[i]);
					break;
				}
			}
		}
	}
	return out;
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

void DrawList::shutdown()
{
	for (auto& ptr : m_shape_models)
	{
		delete ptr;
	}
	m_shape_models.clear();
}

/// <summary>
/// Currently, scaling is not supported together with rotation
/// </summary>
void DrawList::draw_all()
{
	for (auto shape : m_shape_models)
	{
		Angel::mat4 model_mat = shape->model_matrix();
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
			m_renderer->draw_triangles(shape->vertex_array(), shape->triangles_index_buffer(), Shape::shader());
			// TODO
			if (shape->is_selected())
			{
				Shape::shader()->set_uniform_4f("u_color",
					0.0f,
					0.0f,
					0.0f,
					1.0f);
				if (shape->shape_def() == ShapeModel::StaticShape::RECTANGLE)
				{
					m_renderer->draw_lines(shape->vertex_array(), shape->triangles_index_buffer(), Shape::shader());
				}
				else
				{
					m_renderer->draw_lines(shape->vertex_array(), shape->triangles_index_buffer(), Shape::shader(), shape->true_num_vertices());
				}
			}
		}
		else
		{
			m_renderer->draw_polygon(shape->vertex_array(), shape->triangles_index_buffer(), Shape::shader());
			if (shape->is_selected())
			{
				Shape::shader()->set_uniform_4f("u_color",
					0.0f,
					0.0f,
					0.0f,
					1.0f);
				unsigned int offset = sizeof(unsigned int); // Polygon IB has offset of 1 to the actual starting vertex (not the center)
				m_renderer->draw_lines(shape->vertex_array(), shape->triangles_index_buffer(), Shape::shader(), shape->true_num_vertices(),(const void*)offset);
			}
		}
	}
}


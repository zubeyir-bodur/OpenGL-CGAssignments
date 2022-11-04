#include "DrawList.h"
#include "ErrorManager.h"
#include "Angel-maths/mat.h"
#include <glew.h>

DrawList::DrawList(const Angel::mat4& proj, const Angel::mat4& view)
{
	m_proj_mat = const_cast<Angel::mat4*>(&proj);
	m_view_mat = const_cast<Angel::mat4*>(&view);
}

/// <summary>
/// Does not do the garbage collection, it 
/// should be done with shutdown instead!
/// </summary>
DrawList::~DrawList()
{
	m_shape_models.clear();
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

/// <summary>
/// Moves a specific shape to the tail of the draw list,
/// so that the draw call for that shape is made last. This way,
/// a 2D shape can be moved to in front of all other shapes, works
/// with orthogonal projection only.
/// </summary>
/// <param name="s"></param>
void DrawList::move_shape_to_frontview(ShapeModel* s)
{
	unsigned int idx = idx_of(s);
	ASSERT(idx != -1);
	m_shape_models.erase(m_shape_models.begin() + idx);
	m_shape_models.push_back(s);
}

/// <summary>
/// Assuming all shapes in the list are 2D, retrieves the 
/// shape at the given 2D world coordinate
/// </summary>
/// <param name="model_pos"></param>
/// <returns></returns>
ShapeModel* DrawList::frontmost_shape_2d(const Angel::vec3& model_pos)
{
	for (int i = m_shape_models.size() - 1; i >= 0; i--)
	{
		if (m_shape_models[i]->contains_2d(model_pos))
		{
			return m_shape_models[i];
		}
	}
	return nullptr;
}

/// <summary>
/// Assuming all shapes in the list are 2D, retrieves the
/// shapes at the given 2D bounding box</summary>
/// <param name="selector_pos"></param>
/// <param name="selector_scale"></param>
/// <returns></returns>
const std::vector<ShapeModel*> DrawList::shapes_contained_in_2d(
	const Angel::vec3& selector_pos, 
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
			if (selection_rectangle_sm.contains_2d(point_j))
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
				if (m_shape_models[i]->contains_2d(point_j))
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

void DrawList::undo_add_predefined(ShapeModel* s)
{
	std::cout << "Undo predefined shape creation" << std::endl;
	unsigned int idx_of_s = idx_of(s);
	m_shape_models[idx_of_s]->is_hidden() = true;
}

void DrawList::redo_add_predefined(ShapeModel* s)
{
	std::cout << "Redo predefined shape creation" << std::endl;
	unsigned int idx_of_s = idx_of(s);
	m_shape_models[idx_of_s]->is_hidden() = false;
}

void DrawList::undo_finish_poly(ShapeModel* s)
{
	std::cout << "Undo polygon finished" << std::endl;
	unsigned int idx_of_s = idx_of(s);
	m_shape_models[idx_of_s]->is_hidden() = true;
}

void DrawList::redo_finish_poly(ShapeModel* s)
{
	std::cout << "Redo polygon finished" << std::endl;
	unsigned int idx_of_s = idx_of(s);
	m_shape_models[idx_of_s]->is_hidden() = false;
}

void DrawList::undo_move(ShapeModel* s, const Angel::vec3& move_amount)
{
	s->position() -= move_amount;
}

void DrawList::redo_move(ShapeModel* s, const Angel::vec3& move_amount)
{
	s->position() += move_amount;
}

void DrawList::undo_rotate(ShapeModel* s, const Angel::vec3& rotate_amount)
{
	s->rotation() -= rotate_amount;
}

void DrawList::redo_rotate(ShapeModel* s, const Angel::vec3& rotate_amount)
{
	s->rotation() += rotate_amount;
}

/// <summary>
/// Must be called only when there is a valid OpenGL context!
/// </summary>
void DrawList::shutdown()
{
	for (auto& ptr : m_shape_models)
	{
		delete ptr;
	}
	m_shape_models.clear();
}

/// <summary>
/// Draws all shape models in the list
/// </summary>
void DrawList::draw_all()
{
	for (auto shape : m_shape_models)
	{
		shape->draw_shape(*m_proj_mat, *m_view_mat);		
	}
}


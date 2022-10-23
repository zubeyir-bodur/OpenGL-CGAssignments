#pragma once
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShapeModel.h"
#include "Angel-maths/mat.h"

class DrawList
{
private:
	std::vector<ShapeModel*> m_shape_models;
	Renderer* m_renderer;
	Angel::mat4* m_proj_mat;
	Angel::mat4* m_view_mat;
public:
	DrawList(Renderer* r, const Angel::mat4& proj, const Angel::mat4& view);
	~DrawList();

	void add_shape(ShapeModel* s);
	void remove_shape(ShapeModel* s);
	void move_shape_to_frontview(ShapeModel* s);

	ShapeModel* frontmost_shape(const Angel::vec3& cursor_model_pos);
	const std::vector<ShapeModel*> shapes_contained_in(const Angel::vec3& selector_pos, const Angel::vec3& selector_scale);
	const std::vector<ShapeModel*>& shape_models() { return m_shape_models; }
	unsigned int idx_of(ShapeModel* s);

	void undo_add_predefined(ShapeModel* s);
	void redo_add_predefined(ShapeModel* s);
	void undo_finish_poly(ShapeModel* s);
	void redo_finish_poly(ShapeModel* s);
	void undo_move(ShapeModel* s, const Angel::vec3& move_amount);
	void redo_move(ShapeModel* s, const Angel::vec3& move_amount);
	void undo_rotate(ShapeModel* s, const Angel::vec3& rotate_amount);
	void redo_rotate(ShapeModel* s, const Angel::vec3& rotate_amount);
	
	void shutdown();
	void draw_all();
};
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
	unsigned int idx_of(ShapeModel* s);
	void shutdown();
	void draw_all();
};
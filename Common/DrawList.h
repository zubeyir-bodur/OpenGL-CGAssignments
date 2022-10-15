#pragma once
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shape.h"
#include <glm/glm.hpp>

class DrawList
{
private:
	std::vector<Shape*> m_shapes;
	Renderer* m_renderer;
	glm::mat4* m_proj_mat;
	glm::mat4* m_view_mat;
public:
	DrawList(Renderer* r, const glm::mat4& proj, const glm::mat4& view);
	~DrawList();

	void add_shape(Shape* s);

	void move_shape_to_frontview(Shape*);

	Shape* frontmost_shape_at_pos(float model_x, float model_y);

	unsigned int idx_of(Shape* s);

	void draw_all();
};
#pragma once
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShapeModel.h"
#include <glm/glm.hpp>

class DrawList
{
private:
	std::vector<ShapeModel*> m_shape_models;
	Renderer* m_renderer;
	glm::mat4* m_proj_mat;
	glm::mat4* m_view_mat;
public:
	DrawList(Renderer* r, const glm::mat4& proj, const glm::mat4& view);
	~DrawList();

	void add_shape(ShapeModel* s);

	void move_shape_to_frontview(ShapeModel* s);

	ShapeModel* frontmost_shape(const glm::vec3& model_pos);

	unsigned int idx_of(ShapeModel* s);

	void draw_all();
};
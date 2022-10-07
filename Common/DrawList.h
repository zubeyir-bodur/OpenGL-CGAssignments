#pragma once
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <glm/glm.hpp>
struct Shape
{
	glm::vec3* m_model_positions;
	float* m_model_colors;
	std::vector<glm::vec2>* m_vertex_coordinates;
	VertexArray* m_vertex_array;
	IndexBuffer* m_index_buffer;
	VertexBufferLayout* m_layout;
	Shader* m_shader;

	bool contains(float model_x, float model_y);
	void on_vertex_coordinates_changed(const std::vector<glm::vec2>& new_coords);
};

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
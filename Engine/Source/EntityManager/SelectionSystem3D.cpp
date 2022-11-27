#include "Core/ErrorManager.h"

#include "EntityManager/SelectionSystem3D.h"
#include <functional>
#include <glew.h>

SelectionSystem3D::SelectionSystem3D(DrawList* draw_list, int width, int height)
{
	// Initialize FrameBuffer
	m_entity_picker_fb = new FrameBuffer(width, height);
	
	// Give access to the DrawList of the app
	m_draw_list = draw_list;

	// Shader for the selection system to write the 
	// DrawList index to this FrameBuffer
	m_picker_shader = new Shader("../../Engine/Shaders/pickable_triangle.glsl");
}

SelectionSystem3D::~SelectionSystem3D()
{
	delete m_entity_picker_fb;
	delete m_picker_shader;
}

unsigned int SelectionSystem3D::on_update(int window_x, int window_y)
{
	m_picker_shader->bind();
	m_entity_picker_fb->on_update([&]() 
		{
			auto& list = m_draw_list->shape_models();
			unsigned int index = 1;
			for (auto shape_model : list)
			{
				std::array<uint32_t, 3> u_shape_model_id = map_drawlist_idx_to_rgb(index);
				m_picker_shader->set_uniform_3ui("u_shape_model_id", 
					u_shape_model_id[0],
					u_shape_model_id[1],
					u_shape_model_id[2]);
				Angel::mat4 mvp = m_draw_list->projection_matrix() * m_draw_list->view_matrix() * shape_model->model_matrix();
				m_picker_shader->set_uniform_mat4f("u_MVP", mvp);
				if (!shape_model->is_poly())
				{
					Renderer::draw_triangles(shape_model->vertex_array(), shape_model->index_buffer(), m_picker_shader);
				}
				else
				{
					Renderer::draw_polygon(shape_model->vertex_array(), shape_model->index_buffer(), m_picker_shader);
				}
				index++;
			}
			
		});
	// Do the work for unbinding the frame buffer, depth buffer & texture here
	unsigned int cur_idx = get_entity_idx_at(window_x, window_y);
	m_entity_picker_fb->unbind();
	m_picker_shader->unbind();
	auto viewport_size = m_entity_picker_fb->viewport_size();
	__glCallVoid(glViewport(0, 0, viewport_size[0], viewport_size[1]));
	__glCallVoid(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	return cur_idx;
}

void SelectionSystem3D::on_screen_resize(int new_width, int new_height)
{
	m_entity_picker_fb->on_screen_resize(new_width, new_height);
}

unsigned int SelectionSystem3D::get_entity_idx_at(int window_x, int window_y)
{
	std::array<uint32_t, 3> pixel = m_entity_picker_fb->read_pixel(window_x, window_y);
	return map_rgb_to_drawlist_idx(pixel);
}

/// <summary>
/// 
/// </summary>
/// <param name="index">1 indexed index of the entity of the drawlist</param>
/// <returns>fragment color of the entity id to be drawn in the FB</returns>
std::array<uint32_t, 3> SelectionSystem3D::map_drawlist_idx_to_rgb(uint32_t index)
{
	uint32_t r = static_cast<uint32_t>((index >>  0) & 255);
	uint32_t g = static_cast<uint32_t>((index >>  8) & 255);
	uint32_t b = static_cast<uint32_t>((index >> 16) & 255);
	return {r, g, b};
}

/// <summary>
/// 
/// </summary>
/// <param name="rgba">fragment color of the entity id to be drawn in the FB</param>
/// <returns>1 indexed index of the entity of the draw list</returns>
uint32_t SelectionSystem3D::map_rgb_to_drawlist_idx(const std::array<uint32_t, 3>& rgba)
{
	uint32_t r = rgba[0] <<  0;
	uint32_t g = rgba[1] <<  8;
	uint32_t b = rgba[2] << 16;
	return r + g + b;
}

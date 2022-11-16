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
}

void SelectionSystem3D::on_update()
{
	m_entity_picker_fb->on_update([=]() 
		{
			auto& list = m_draw_list->shape_models();
			unsigned int index = 1;
			for (auto shape_model : list)
			{
				// TODO parse & set uniform
				std::array<uint8_t, 4> u_shape_model_id = map_drawlist_idx_to_rgba(index);
				m_picker_shader->set_uniform_4f("u_shape_model_id", 0.5f, 0.5f, 0.5f, 1.0f);

				m_picker_shader->set_uniform_mat4f("u_MVP", shape_model->model_matrix());
				auto shape_model_def = shape_model->shape_def();
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
	m_entity_picker_fb->unbind();
	auto viewport_size = m_entity_picker_fb->viewport_size();
	__glCallVoid(glViewport(0, 0, viewport_size[0], viewport_size[1]));
	__glCallVoid(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

}

void SelectionSystem3D::on_screen_resize(int new_width, int new_height)
{
	m_entity_picker_fb->on_screen_resize(new_width, new_height);
}

// TODO : Implement
std::array<uint8_t, 4> SelectionSystem3D::map_drawlist_idx_to_rgba(unsigned int index)
{
	return {};
}

// TODO : Implement
unsigned int SelectionSystem3D::map_rgba_to_drawlist_idx(const std::array<uint8_t, 4>& rgba)
{
	return 0;
}

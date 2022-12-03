#pragma once
#include "Renderer/FrameBuffer.h"

#include "EntityManager/DrawList.h"
#include "EntityManager/ArticulatedModel.h"

class SelectionSystem3D
{
private:
	DrawList* m_draw_list = nullptr;
	ArticulatedModel* m_hierarchical_model = nullptr;
	Shader* m_picker_shader = nullptr;
	FrameBuffer* m_entity_picker_fb = nullptr;

	/// <summary>
	/// Warning : A valid ShapeModel's index starts from 1
	/// 0 index is reserved for the clear color
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	static std::array<uint8_t, 3> map_drawlist_idx_to_rgb(unsigned int index);
	static unsigned int map_rgb_to_drawlist_idx(const std::array<uint8_t, 3>& rgb);
	unsigned int get_entity_idx_at(int window_x, int window_y);
public:
	SelectionSystem3D() = default;
	SelectionSystem3D(DrawList* draw_list, ArticulatedModel* model, int width, int height);
	~SelectionSystem3D();

	unsigned int on_update(int window_x, int window_y);
	void on_screen_resize(int new_width, int new_height);

};
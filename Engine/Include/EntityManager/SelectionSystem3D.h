#pragma once
#include "Renderer/FrameBuffer.h"

#include "EntityManager/DrawList.h"

class SelectionSystem3D
{
private:
	DrawList* m_draw_list = nullptr;
	Shader* m_picker_shader = nullptr;
	FrameBuffer* m_entity_picker_fb = nullptr;
public:
	SelectionSystem3D() = default;
	SelectionSystem3D(DrawList* draw_list, int width, int height);
	~SelectionSystem3D();

	void on_update();
	void on_screen_resize(int new_width, int new_height);

	/// <summary>
	/// Warning : A valid ShapeModel's index starts from 1
	/// 0 index is reserved for the clear color
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	static std::array<uint8_t, 4> map_drawlist_idx_to_rgba(unsigned int index);
	static unsigned int map_rgba_to_drawlist_idx(const std::array<uint8_t, 4>& rgba);
};
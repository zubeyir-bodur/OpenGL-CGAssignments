#pragma once
#include "Core/ErrorManager.h"
#include <cstdint>

// forward declare function
namespace std {
	template <class T>
	class function;

	template <class T, size_t N>
	class array;
}

class FrameBuffer
{
private:
	unsigned int m_frame_buffer_id;  // The frame buffer object
	unsigned int m_fb_texture_id;	// The texture to be displayed in this frame buffer
	unsigned int m_fb_depth_buffer_id;	// Depth buffer of this FB
	int m_viewport_width, m_viewport_height; // Cached viewport size of this FB
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();

	void on_update(const std::function<void(void)>& draw_function);
	void on_screen_resize(int new_width, int new_height);
	void bind();
	void unbind();
	std::array<uint32_t, 3> read_pixel(int pixel_x, int pixel_y);
	std::array<int, 2> viewport_size();

};
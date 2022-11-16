#include "Renderer/FrameBuffer.h"
#include <glew.h>
#include <array>
#include <functional>

FrameBuffer::FrameBuffer(int width, int height)
	: m_viewport_width(width),
	m_viewport_height(height)
{
	// Create Texture for FB 
	__glCallVoid(glGenTextures(1, &m_fb_texture_id));
	__glCallVoid(glBindTexture(GL_TEXTURE_2D, m_fb_texture_id));
	__glCallVoid(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	__glCallVoid(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	__glCallVoid(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	// Create Depth render buffer for FB
	__glCallVoid(glGenRenderbuffers(1, &m_fb_depth_buffer_id));
	__glCallVoid(glBindRenderbuffer(GL_RENDERBUFFER, m_fb_depth_buffer_id));

	// Create the actual frame buffer
	__glCallVoid(glGenFramebuffers(1, &m_frame_buffer_id));
	__glCallVoid(glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_id));

	// Attach the texture as the first color attachment
	__glCallVoid(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fb_texture_id, 0));

	// Make a depth buffer and the same size as the targetTexture
	__glCallVoid(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fb_depth_buffer_id));
}

FrameBuffer::~FrameBuffer()
{
	// Delete FB and their attachments
	__glCallVoid(glDeleteTextures(1, &m_fb_texture_id));
	__glCallVoid(glDeleteRenderbuffers(1, &m_fb_depth_buffer_id));
	__glCallVoid(glDeleteFramebuffers(1, &m_frame_buffer_id));
}

void FrameBuffer::on_update(const std::function<void(void)>& draw_function)
{
	bind();
	__glCallVoid(glViewport(0, 0, m_viewport_width, m_viewport_height));

	__glCallVoid(glEnable(GL_CULL_FACE));
	__glCallVoid(glEnable(GL_DEPTH_TEST));

	// Clear the canvas AND the depth buffer
	__glCallVoid(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Draw whatever needs to be drawn to this frame buffer
	// Which is specified in the lambda
	draw_function();
}

void FrameBuffer::on_screen_resize(int new_width, int new_height)
{
	m_viewport_width = new_width;
	m_viewport_height = new_height;
	__glCallVoid(glBindTexture(GL_TEXTURE_2D, m_fb_texture_id));

	// Level = 0, Border = 0, RGBA8 - same as the Texture class
	// But the data is mull
	// This way, an empty texture will be reinitialized to our bound texture
	__glCallVoid(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
		m_viewport_width, m_viewport_height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

	__glCallVoid(glBindRenderbuffer(GL_RENDERBUFFER, m_fb_depth_buffer_id));
	__glCallVoid(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_viewport_width, m_viewport_height));
}

void FrameBuffer::bind()
{
	__glCallVoid(glBindTexture(GL_TEXTURE_2D, m_fb_texture_id));
	__glCallVoid(glBindRenderbuffer(GL_RENDERBUFFER, m_fb_depth_buffer_id));
	__glCallVoid(glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer_id));
}

void FrameBuffer::unbind()
{
	__glCallVoid(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	// Also unbinds the texture used & depth buffer in this FB
	__glCallVoid(glBindTexture(GL_TEXTURE_2D, 0));
	__glCallVoid(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

std::array<uint8_t, 4> FrameBuffer::read_pixel(int pixel_x, int pixel_y)
{
	bind();
	uint8_t* data = nullptr;
	__glCallVoid(glReadPixels(
		pixel_x,			// x
		pixel_y,			// y
		1,					// width
		1,					// height
		GL_RGBA,			// format
		GL_UNSIGNED_BYTE,	// type
		(void*)data));		// typed array to hold result
	return 
	{
		data[0],
		data[1],
		data[2],
		data[3]
	};
}

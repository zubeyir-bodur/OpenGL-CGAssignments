#include "Texture.h"
#include "ErrorManager.h"
#include "glew.h"
#include "nothings-stb/stb_image.h"

Texture::Texture(const std::string& path) :
	m_file_path(path), m_width(0), m_height(0),
	m_texture_id(0), m_local_buffer(nullptr), m_bytes_per_pixel(0)
{
	stbi_set_flip_vertically_on_load(false);
	m_local_buffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bytes_per_pixel, 4);

	__glCallVoid(glGenTextures(1, &m_texture_id));
	__glCallVoid(glBindTexture(GL_TEXTURE_2D, m_texture_id));

	__glCallVoid(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	__glCallVoid(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	__glCallVoid(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	__glCallVoid(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	__glCallVoid(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_local_buffer));
	unbind();
	if (m_local_buffer)
	{
		stbi_image_free(m_local_buffer);
	}
}

Texture::~Texture()
{
	__glCallVoid(glDeleteTextures(1, &m_texture_id));
}

void Texture::bind(unsigned int slot_number /*= 0*/) const
{
	__glCallVoid(glActiveTexture(GL_TEXTURE0 + slot_number));
	__glCallVoid(glBindTexture(GL_TEXTURE_2D, m_texture_id));
}

void Texture::unbind() const
{
	__glCallVoid(glBindTexture(GL_TEXTURE_2D, 0));
}

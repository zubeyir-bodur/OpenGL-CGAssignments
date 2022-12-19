#include "Renderer/BumpMap.h"
#include <Core/ErrorManager.h>
#include <nothings-stb/stb_image.h>

BumpMap::BumpMap(const std::string& path) : m_map_filepath(path)
{
	stbi_set_flip_vertically_on_load(true);
	int l_bpp;
	unsigned char* l_buffer = stbi_load(m_map_filepath.data(), &m_width, &m_height, &l_bpp, 4);

	m_displacement_derivative = new Angel::vec2*[m_width];
	for (int i = 0; i < m_width; i++)
	{
		m_displacement_derivative[i] = new Angel::vec2[m_height];
		for (int j = 0; j < m_height; j++)
		{
			// TODO
			m_displacement_derivative[i][j] = { 0.0f, 0.0f };
		}
	}
	if (l_buffer)
	{
		stbi_image_free(l_buffer);
	}
}

BumpMap::~BumpMap()
{
	for (int i = 0; i < m_width; i++)
	{
		delete[] m_displacement_derivative[i];
	}
	delete[] m_displacement_derivative;
}

float BumpMap::del_d_del_u(float u, float v)
{
	ASSERT(u >= 0.0f && u <= 1.0f);
	ASSERT(v >= 0.0f && v <= 1.0f);
	unsigned int i = u * (m_width - 1);
	unsigned int j = v * (m_height - 1);
	return m_displacement_derivative[i][j].x;
}

float BumpMap::del_d_del_v(float u, float v)
{
	ASSERT(u >= 0.0f && u <= 1.0f);
	ASSERT(v >= 0.0f && v <= 1.0f);
	unsigned int i = u * (m_width - 1);
	unsigned int j = v * (m_height - 1);
	return m_displacement_derivative[i][j].y;
}

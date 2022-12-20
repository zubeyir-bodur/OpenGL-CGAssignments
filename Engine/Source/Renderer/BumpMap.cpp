#include "Renderer/BumpMap.h"
#include <Core/ErrorManager.h>
#include <nothings-stb/stb_image.h>

BumpMap::BumpMap(const std::string& path) : m_map_filepath(path)
{
	stbi_set_flip_vertically_on_load(true);
	// Only gray channel is needed
	unsigned char* l_buffer = stbi_load(m_map_filepath.data(), &m_width, &m_height, &m_bpp, 1);

	m_displacement_derivative = new Angel::vec2*[m_width];
	for (int i = 0; i < m_width; i++)
	{
		m_displacement_derivative[i] = new Angel::vec2[m_height];
		for (int j = 0; j < m_height; j++)
		{
			unsigned char d_i_j = l_buffer[j * m_width + i];

			unsigned char d_i1_j;
			if (i+1 < m_width)
			{
				d_i1_j = l_buffer[j * m_width + i + 1];
			}
			else
			{
				d_i1_j = 0;
			}

			unsigned char d_i_j1;
			if (j+1 < m_height)
			{
				d_i_j1 = l_buffer[(j+1) * m_width + i];
			}
			else
			{
				d_i_j1 = 0;
			}

			// Normalize the derivative between 0 & 1
			m_displacement_derivative[i][j] = Angel::vec2(
					d_i_j - d_i1_j, 
					d_i_j - d_i_j1) / 255.0f;
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
	unsigned int i = std::roundf(u * (m_width - 1));
	unsigned int j = std::roundf(v * (m_height - 1));
	if (i < 0) i = 0; if (i >= m_width) i = m_width - 1;
	if (j < 0) j = 0; if (j >= m_height) j = m_height - 1;
	return m_displacement_derivative[i][j].x;
}

float BumpMap::del_d_del_v(float u, float v)
{
	ASSERT(u >= 0.0f && u <= 1.0f);
	ASSERT(v >= 0.0f && v <= 1.0f);
	unsigned int i = u * (m_width - 1);
	unsigned int j = v * (m_height - 1);
	if (i < 0) i = 0; if (i >= m_width) i = m_width - 1;
	if (j < 0) j = 0; if (j >= m_height) j = m_height - 1;
	return m_displacement_derivative[i][j].y;
}

#pragma once
#include "Angel-maths/mat.h"
#include <string>

class BumpMap
{
private:
	std::string m_map_filepath;
	int m_width, m_height, m_bpp;
	Angel::vec2** m_displacement_derivative;
public:
	BumpMap(const std::string& path);
	~BumpMap();

	float del_d_del_u(float u, float v);
	float del_d_del_v(float u, float v);

};
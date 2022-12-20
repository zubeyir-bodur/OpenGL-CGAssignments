#pragma once
#include "Angel-maths/mat.h"
#include <string>
#include "Renderer/Texture.h"

class BumpMap
{
private:
	std::string m_map_filepath;
	int m_width, m_height, m_bpp;
	unsigned char* m_displacement_derivative;
	Texture* m_bump_texture;
public:
	BumpMap(const std::string& path);
	~BumpMap();

	float del_d_del_u(float u, float v);
	float del_d_del_v(float u, float v);
	inline const Texture* bump_texture() const { return m_bump_texture; }

};
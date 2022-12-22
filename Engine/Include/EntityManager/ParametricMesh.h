#pragma once

#include "Angel-maths/mat.h"

#include "Renderer/Shader.h"
#include "Renderer/VertexBufferLayout.h"
#include "Renderer/VertexArray.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/BumpMap.h"

#include <vector>

class ParametricMesh
{
public:
	enum class DisplayType
	{
		Wireframe,
		Gouraud,
		Phong
	};
private:
	// Mesh Data -> 50-1x50-1x11 mesh data (u, v, element, coordinate)
	Angel::vec3** m_mesh_points;
	Angel::vec3** m_mesh_normals;
	float* m_mesh_buffer_data;

	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;
	IndexBuffer* m_wireframe_ibo;
	BumpMap* m_bumpmap;

	// Albedo color
	Angel::vec4 m_color;

	// Lighting parameters
	Angel::vec4 m_ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
	Angel::vec4 m_diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	Angel::vec4 m_specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	float m_shininess = 50.0f;

	// Curve parameters
	float m_R, m_r, m_l, m_q;
	unsigned int m_row_subdiv, m_col_subdiv, m_old_row_subdiv, m_old_col_subdiv;

	bool m_just_changed;

	static Shader* s_g_shader; // Gouraud Shading
	static Shader* s_p_shader; // Phong Shading
	static Shader* s_wireframe_shader;
	static VertexBufferLayout* s_parametric_mesh_layout;

	void construct_mesh();
public:

	ParametricMesh(float R, float r, float l,  float q,
		unsigned int row_subdiv, unsigned int col_subdiv, 
		const Angel::vec4& col, BumpMap* bumpmap);
	~ParametricMesh();
	
	void update_and_draw(
		const Angel::mat4& proj, 
		const Angel::mat4& view, 
		DisplayType display_type);
	void set_R(float R);
	void set_r(float r);
	void set_l(float l);
	void set_q(float q);
	void set_row_subdiv(unsigned int r_sbd);
	void set_col_subdiv(unsigned int c_sbd);
	inline Angel::vec4& color() { return m_color; }
	inline Angel::vec4& ambient() { return m_ambient; }
	inline Angel::vec4& diffuse() { return m_diffuse; }
	inline Angel::vec4& specular() { return m_specular; }
	inline float& shininess() { return m_shininess; }

	static void init_static_members();
	static void destroy_static_members();
};
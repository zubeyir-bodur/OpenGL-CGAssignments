#pragma once

#include "Angel-maths/mat.h"

#include "Renderer/Shader.h"
#include "Renderer/VertexBufferLayout.h"
#include "Renderer/VertexArray.h"
#include "Renderer/IndexBuffer.h"

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
	// Mesh Data -> 50-1x50-1x2x3 mesh data (u, v, element, coordinate)
	Angel::vec3** m_mesh_points;
	Angel::vec3** m_mesh_normals;
	std::vector<float> m_mesh_buffer_data;

	VertexArray* m_vao;
	VertexBuffer* m_vbo;

	// Albedo color
	Angel::vec4 m_color;

	// Lighting parameters
	Angel::vec4 m_ambient = { 0.32f, 0.173f, 0.118f, 1.0f };
	Angel::vec4 m_diffuse = { 0.75f, 0.5f, 0.0f, 1.0f };
	Angel::vec4 m_specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	float m_shininess = 50.0f;

	// Curve parameters
	float m_R, m_r, m_l, m_q;

	bool m_just_changed;

	static Shader* s_g_shader; // Gouraud Shading
	static Shader* s_p_shader; // Phong Shading
	static Shader* s_wireframe_shader;
	static VertexBufferLayout* s_parametric_mesh_layout;
	static IndexBuffer* s_ibo;
	static IndexBuffer* s_wireframe_ibo;

	void construct_mesh(float R, float r, float l, float q);
public:

	ParametricMesh(float R, float r, float l,  float q, 
		const Angel::vec4& col);
	~ParametricMesh();
	
	void update_and_draw(
		const Angel::mat4& proj, 
		const Angel::mat4& view, 
		DisplayType display_type);
	inline void set_R(float R) { if (R != m_R) m_just_changed = true; m_R = R; }
	inline void set_r(float r) { if (r != m_r) m_just_changed = true; m_r = r; }
	inline void set_l(float l) { if (l != m_l) m_just_changed = true; m_l = l; }
	inline void set_q(float q) { if (q != m_q) m_just_changed = true; m_q = q; }
	inline Angel::vec4& color() { return m_color; }
	inline Angel::vec4& ambient() { return m_ambient; }
	inline Angel::vec4& diffuse() { return m_diffuse; }
	inline Angel::vec4& specular() { return m_specular; }
	inline float& shininess() { return m_shininess; }

	static void init_static_members();
	static void destroy_static_members();
};
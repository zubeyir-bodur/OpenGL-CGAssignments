#include "EntityManager/ParametricMesh.h"

Shader* ParametricMesh::s_shader = nullptr;
Shader* ParametricMesh::s_wireframe_shader = nullptr;
VertexBufferLayout* ParametricMesh::s_parametric_mesh_layout = nullptr;
IndexBuffer* ParametricMesh::s_ibo = nullptr;

/// <summary>
/// Constructs the mesh. If its already constructed, it reconstructs w.r.t new parameters
/// </summary>
void ParametricMesh::construct_mesh(float R, float r, float l, float q)
{
	// TODO
}

ParametricMesh::ParametricMesh(float R, float r, float l, float q, const Angel::vec3& col)
{
	m_R = R; m_r = r; m_l = l; m_q = q; m_color = col;

	construct_mesh(m_R, m_r, m_l, m_q);

}

ParametricMesh::~ParametricMesh()
{

}

void ParametricMesh::update_and_draw()
{
	if (m_just_changed)
	{
		construct_mesh(m_R, m_r, m_l, m_q);
		m_just_changed = false;
	}

	// Draw
}

void ParametricMesh::init_static_members()
{

}

void ParametricMesh::destroy_static_members()
{

}

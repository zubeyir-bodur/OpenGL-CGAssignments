#include "EntityManager/ParametricMesh.h"

#include "Renderer/Renderer.h"

#define NUM_ROWS 50
#define NUM_COLUMNS 50
#define NUM_MESH_ELEMENTS 2
#define NUM_MESH_COORDINATES 3

#define NUM_VERTICES_PER_QUAD 4
#define NUM_INDICES_PER_QUAD 6
#define NUM_WIREFRAME_INDICES_PER_QUAD 8

#define NUM_QUADS (NUM_ROWS-1) * (NUM_COLUMNS-1)

#define NUM_VERTICES NUM_QUADS * NUM_VERTICES_PER_QUAD
#define NUM_INDICES NUM_QUADS * NUM_INDICES_PER_QUAD
#define NUM_WIREFRAME_INDICES NUM_QUADS * NUM_WIREFRAME_INDICES_PER_QUAD

Shader* ParametricMesh::s_g_shader = nullptr;
Shader* ParametricMesh::s_p_shader = nullptr;
Shader* ParametricMesh::s_wireframe_shader = nullptr;
VertexBufferLayout* ParametricMesh::s_parametric_mesh_layout = nullptr;
IndexBuffer* ParametricMesh::s_ibo = nullptr;
IndexBuffer* ParametricMesh::s_wireframe_ibo = nullptr;

/// <summary>
/// Constructs the mesh. If its already constructed, it reconstructs w.r.t new parameters
/// </summary>
void ParametricMesh::construct_mesh(float R, float r, float l, float q)
{
	if (m_vao != nullptr)
	{
		delete m_vao;
	}
	if (m_vbo != nullptr)
	{
		delete m_vbo;
	}
	// TODO
	
}

ParametricMesh::ParametricMesh(float R, float r, float l, float q, const Angel::vec4& col)
{
	m_R = R; m_r = r; m_l = l; m_q = q; m_color = col;
	m_just_changed = false;
	m_vao = nullptr;
	m_vbo = nullptr;
	m_mesh_points = {};
	m_mesh_normals = {};
	m_mesh_buffer_data = {};
	construct_mesh(m_R, m_r, m_l, m_q);
}

ParametricMesh::~ParametricMesh()
{
	delete m_vao;
	delete m_vbo;
}

void ParametricMesh::update_and_draw(
	const Angel::mat4& proj, 
	const Angel::mat4& view, 
	DisplayType display_type)
{
	if (m_just_changed)
	{
		construct_mesh(m_R, m_r, m_l, m_q);
		m_just_changed = false;
	}

	// Draw
	if (display_type == ParametricMesh::DisplayType::Wireframe)
	{
		Angel::mat4 MVP_matrix = proj * view;
		s_wireframe_shader->bind();
		s_wireframe_shader->set_uniform_mat4f("u_MVP", MVP_matrix);
		s_wireframe_shader->set_uniform_4f("u_color",
			m_color.x,
			m_color.y,
			m_color.z,
			m_color.w);
		Renderer::draw_seperate_lines(m_vao, s_wireframe_ibo, s_wireframe_shader);
	}
	else
	{
		Shader* shader_to_use = nullptr;
		if (display_type == ParametricMesh::DisplayType::Gouraud)
		{
			shader_to_use = s_g_shader;
		}
		else if (display_type == ParametricMesh::DisplayType::Phong)
		{
			shader_to_use = s_p_shader;
		}
		else
		{
			return;
		}
		shader_to_use->bind();
		shader_to_use->set_uniform_mat4f("u_MV", view);
		shader_to_use->set_uniform_mat4f("u_P", proj);
		Angel::vec4 u_light_position(0.0f, 1000.0f, 1000.0f, 1.0f);
		shader_to_use->set_uniform_4f("u_light_position",
			u_light_position.x,
			u_light_position.y,
			u_light_position.z,
			u_light_position.w);
		shader_to_use->set_uniform_4f("u_color",
			m_color.x,
			m_color.y,
			m_color.z,
			m_color.w);
		shader_to_use->set_uniform_4f("u_ambient", 0.32f, 0.173f, 0.118f, 1.0f);
		shader_to_use->set_uniform_4f("u_diffuse", 0.75f, 0.5f, 0.0f, 1.0f);
		shader_to_use->set_uniform_4f("u_specular", 1.0f, 1.0f, 1.0f, 1.0f);
		shader_to_use->set_uniform_1f("u_shininess", 50.0f);
		Renderer::draw_triangles(m_vao, s_ibo, shader_to_use);
	}
}

void ParametricMesh::init_static_members()
{
	s_g_shader = new Shader("../../Engine/Shaders/g_shaded_triangle.glsl");
	s_p_shader = new Shader("../../Engine/Shaders/p_shaded_triangle.glsl");
	s_wireframe_shader = new Shader("../../Engine/Shaders/normal_triangle.glsl");

	s_parametric_mesh_layout = new VertexBufferLayout;
	s_parametric_mesh_layout->push_back_elements<float>(NUM_MESH_COORDINATES);
	s_parametric_mesh_layout->push_back_elements<float>(NUM_MESH_COORDINATES);

	std::vector<unsigned int> l_indices = {};
	l_indices.reserve(NUM_INDICES);
	for (unsigned int i = 0; i < NUM_QUADS; i++)
	{
		l_indices.emplace_back(i * NUM_VERTICES_PER_QUAD);
		l_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 1);
		l_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 2);

		l_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 2);
		l_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 3);
		l_indices.emplace_back(i * NUM_VERTICES_PER_QUAD);
	}
	s_ibo = new IndexBuffer(l_indices.data(), l_indices.size());

	std::vector<unsigned int> l_wireframe_indices = {};
	l_wireframe_indices.reserve(NUM_WIREFRAME_INDICES);
	for (unsigned int i = 0; i < NUM_QUADS; i++)
	{
		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD);
		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 1);

		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 1);
		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 2);

		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 2);
		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 3);

		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD + 3);
		l_wireframe_indices.emplace_back(i * NUM_VERTICES_PER_QUAD);
	}
	s_wireframe_ibo = new IndexBuffer(l_wireframe_indices.data(), l_wireframe_indices.size());

	s_ibo->unbind();
	s_wireframe_ibo->unbind();
	s_g_shader->unbind();
	s_p_shader->unbind();
	s_wireframe_shader->unbind();
}

void ParametricMesh::destroy_static_members()
{
	delete s_g_shader;
	delete s_p_shader;
	delete s_wireframe_shader;
	delete s_ibo;
	delete s_wireframe_ibo;
	delete s_parametric_mesh_layout;
}

#include "EntityManager/ParametricMesh.h"

#include "Renderer/Renderer.h"

#define NUM_ROWS m_row_subdiv
#define NUM_COLUMNS m_col_subdiv
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

/// <summary>
/// Constructs the mesh. If its already constructed, it reconstructs w.r.t new parameters
/// </summary>
void ParametricMesh::construct_mesh()
{
	if (m_vao != nullptr)
	{
		delete m_vao;
	}
	if (m_vbo != nullptr)
	{
		delete m_vbo;
	}
	if (m_mesh_points != nullptr)
	{
		for (unsigned int i = 0; i < m_old_row_subdiv; i++)
		{
			delete[] m_mesh_points[i];
		}
		delete[] m_mesh_points;
	}
	if (m_mesh_normals != nullptr)
	{
		for (unsigned int i = 0; i < m_old_row_subdiv; i++)
		{
			delete[] m_mesh_normals[i];
		}
		delete[] m_mesh_normals;
	}
	if (m_mesh_buffer_data.capacity() != 0)
	{
		m_mesh_buffer_data = {};
	}
	m_mesh_points = new Angel::vec3*[m_row_subdiv];
	m_mesh_normals = new Angel::vec3*[m_row_subdiv];
	for (unsigned int i = 0; i < m_row_subdiv; i++)
	{
		m_mesh_points[i] = new Angel::vec3[m_col_subdiv];
		m_mesh_normals[i] = new Angel::vec3[m_col_subdiv];

		for (unsigned int j = 0; j < m_col_subdiv; j++)
		{
			float u = i / (float)(m_row_subdiv -1);
			float v = j / (float)(m_col_subdiv -1);

			m_mesh_points[i][j].x = (m_R + m_r * std::cosf(v))
				* (std::cosf(m_l * u)) * (3 * u / (2 * M_PI));
			m_mesh_points[i][j].y = (m_R + m_r * std::cosf(v))
				* (std::sinf(m_l * u)) * (3 * u / (2 * M_PI));
			m_mesh_points[i][j].z = 3 * u * m_r * std::sinf(v)
				+ u * std::sinf(2 * m_q * u) * std::cosf(2 * v);

			Angel::vec3 del_p_del_u, del_p_del_v;

			del_p_del_u.x = (3 / (2 * M_PI))
				* (m_R + m_r * std::cosf(v))
				* (std::cosf(m_l * u) - m_l * u * std::sinf(m_l * u));
			del_p_del_u.y = (3 / (2 * M_PI))
				* (m_R + m_r * std::cosf(v))
				* (std::sinf(m_l * u) + m_l * u * std::cosf(m_l * u));
			del_p_del_u.z = 3 * m_r * std::sinf(v) + std::cosf(2 * v) * (2 * m_q * u * std::cosf(2 * m_q * u) + std::sinf(2 * m_q * u));

			del_p_del_v.x = -3 * m_r * u * std::sinf(v) * std::cosf(m_l * u) / (2 * M_PI);
			del_p_del_v.y = -3 * m_r * u * std::sinf(v) * std::sinf(m_l * u) / (2 * M_PI);
			del_p_del_v.z = u * (3 * m_r * std::cosf(v) - 2 * std::sinf(2 * v) * std::sinf(2 * m_q * u));

			m_mesh_normals[i][j] = Angel::normalize(Angel::cross(del_p_del_u, del_p_del_v));

			// Process the bumpmap
			if (m_bumpmap)
			{
				const Angel::vec3& n = m_mesh_normals[i][j];
				Angel::vec3 perturbed_normal = n
					+ Angel::cross(m_bumpmap->del_d_del_u(u, v) * n, del_p_del_v)
					+ Angel::cross(m_bumpmap->del_d_del_v(u, v) * n, del_p_del_u);
				m_mesh_normals[i][j] = Angel::normalize(perturbed_normal);
			}
		}
	}
	m_mesh_buffer_data.reserve(NUM_VERTICES * NUM_MESH_ELEMENTS * NUM_MESH_COORDINATES);
	for (unsigned int i = 0; i < m_row_subdiv - 1; i++)
	{
		for (unsigned int j = 0; j < m_col_subdiv - 1; j++)
		{
			// VERTEX 0
			// Points
			m_mesh_buffer_data.emplace_back(m_mesh_points[i][j].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_points[i][j].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_points[i][j].z); // Z

			// Normals
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i][j].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i][j].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i][j].z); // Z


			// VERTEX 1
			// Points
			m_mesh_buffer_data.emplace_back(m_mesh_points[i + 1][j].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_points[i + 1][j].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_points[i + 1][j].z); // Z

			// Normals
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i + 1][j].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i + 1][j].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i + 1][j].z); // Z


			// VERTEX 2
			// Points
			m_mesh_buffer_data.emplace_back(m_mesh_points[i + 1][j + 1].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_points[i + 1][j + 1].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_points[i + 1][j + 1].z); // Z

			// Normals
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i + 1][j + 1].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i + 1][j + 1].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i + 1][j + 1].z); // Z


			// VERTEX 3
			// Points
			m_mesh_buffer_data.emplace_back(m_mesh_points[i][j + 1].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_points[i][j + 1].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_points[i][j + 1].z); // Z

			// Normals
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i][j + 1].x); // X
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i][j + 1].y); // Y
			m_mesh_buffer_data.emplace_back(m_mesh_normals[i][j + 1].z); // Z
		}
	}
	m_vbo = new VertexBuffer(m_mesh_buffer_data.data(), m_mesh_buffer_data.size() * sizeof(float));
	m_vao = new VertexArray;
	m_vao->add_buffer(*m_vbo, *s_parametric_mesh_layout);

	if (m_old_row_subdiv != m_row_subdiv ||
		m_old_col_subdiv != m_col_subdiv)
	{
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
		delete m_ibo;
		m_ibo = new IndexBuffer(l_indices.data(), (unsigned int)l_indices.size());

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
		delete m_wireframe_ibo;
		m_wireframe_ibo = new IndexBuffer(l_wireframe_indices.data(), (unsigned int)l_wireframe_indices.size());

		m_ibo->unbind();
		m_wireframe_ibo->unbind();
		m_old_col_subdiv = m_col_subdiv;
		m_old_row_subdiv = m_row_subdiv;
	}
	m_vao->unbind();
	m_vbo->unbind();
	m_just_changed = false;
}

ParametricMesh::ParametricMesh(float R, float r, float l, float q, unsigned int row_subdiv, unsigned int col_subdiv, const Angel::vec4& col, BumpMap* bumpmap)
{
	m_R = R; m_r = r; m_l = l; m_q = q; m_color = col; m_bumpmap = bumpmap; 
	m_row_subdiv = row_subdiv; m_col_subdiv = col_subdiv;
	m_old_row_subdiv = 0; m_old_col_subdiv = 0;
	m_just_changed = false;
	m_vao = nullptr;
	m_vbo = nullptr;
	m_mesh_points = nullptr;
	m_mesh_normals = nullptr;
	m_mesh_buffer_data = {};
	construct_mesh();
}

ParametricMesh::~ParametricMesh()
{
	if (m_vao != nullptr)
	{
		delete m_vao;
	}
	if (m_vbo != nullptr)
	{
		delete m_vbo;
	}
	if (m_ibo != nullptr)
	{
		delete m_ibo;
	}
	if (m_wireframe_ibo != nullptr)
	{
		delete m_wireframe_ibo;
	}
	if (m_mesh_points != nullptr)
	{
		for (unsigned int i = 0; i < m_row_subdiv; i++)
		{
			delete[] m_mesh_points[i];
		}
		delete[] m_mesh_points;
	}
	if (m_mesh_normals != nullptr)
	{
		for (unsigned int i = 0; i < m_row_subdiv; i++)
		{
			delete[] m_mesh_normals[i];
		}
		delete[] m_mesh_normals;
	}
}

void ParametricMesh::update_and_draw(
	const Angel::mat4& proj, 
	const Angel::mat4& view, 
	DisplayType display_type)
{
	if (m_just_changed)
	{
		construct_mesh();
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
		Renderer::draw_seperate_lines(m_vao, m_wireframe_ibo, s_wireframe_shader);
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
		shader_to_use->set_uniform_4f("u_ambient", m_ambient.x, m_ambient.y, m_ambient.z, m_ambient.w);
		shader_to_use->set_uniform_4f("u_diffuse", m_diffuse.x, m_diffuse.y, m_diffuse.z, m_diffuse.w);
		shader_to_use->set_uniform_4f("u_specular", m_specular.x, m_specular.y, m_specular.z, m_specular.w);
		shader_to_use->set_uniform_1f("u_shininess", m_shininess);
		Renderer::draw_triangles(m_vao, m_ibo, shader_to_use);
	}
}

void ParametricMesh::set_R(float R)
{
	if (R != m_R) 
	{
		m_just_changed = true;
		m_R = R;
	}
}

void ParametricMesh::set_r(float r)
{
	if (r != m_r)
	{
		m_just_changed = true;
		m_r = r;
	}
}

void ParametricMesh::set_l(float l)
{
	if (l != m_l)
	{
		m_just_changed = true;
		m_l = l;
	}
	
}

void ParametricMesh::set_q(float q)
{
	if (q != m_q)
	{
		m_just_changed = true;
		m_q = q;
	}
}

void ParametricMesh::set_row_subdiv(unsigned int r_sbd)
{
	if (r_sbd != m_row_subdiv) 
	{
		m_just_changed = true; 
		m_old_row_subdiv = m_row_subdiv;
		m_row_subdiv = r_sbd;
	}
}

void ParametricMesh::set_col_subdiv(unsigned int c_sbd)
{
	if (c_sbd != m_col_subdiv)
	{
		m_just_changed = true;
		m_old_col_subdiv = m_col_subdiv;
		m_col_subdiv = c_sbd;
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

	s_g_shader->unbind();
	s_p_shader->unbind();
	s_wireframe_shader->unbind();
}

void ParametricMesh::destroy_static_members()
{
	delete s_g_shader;
	delete s_p_shader;
	delete s_wireframe_shader;
	delete s_parametric_mesh_layout;
}

#include "EntityManager/ArticulatedModelNode.h"
#include "Renderer/Renderer.h"
#include "Core/ErrorManager.h"

#include <functional>

ArticulatedModelNode::ArticulatedModelNode(
	const Angel::vec3& scale,
	const Angel::vec3& rotation,
	Texture* texture,
	int texture_slot,
	unsigned int entity_id)
{
	m_cube = new ShapeModel(
		ShapeModel::StaticShape::TEX_CUBE, 
		nullptr, // no position
		nullptr, // no rotation
		new Angel::vec3(scale), 
		texture_slot, 
		texture);
	m_parent = nullptr;
	m_rotation = rotation;
	m_parent_joint_point = {}; // trivial for the torso
	m_entity_id = entity_id;
	m_is_selected = false;
}

ArticulatedModelNode::~ArticulatedModelNode()
{
	destroy_children();
	delete m_cube;
	m_parent = nullptr;
}

Angel::mat4 ArticulatedModelNode::rotation_u()
{
	return (Angel::RotateX(((m_rotation).x))
		* Angel::RotateY(((m_rotation).y))
		* Angel::RotateZ(((m_rotation).z)));
}

Angel::mat4 ArticulatedModelNode::translation_minus_p()
{
	return Angel::Translate(-joint_point());
}

Angel::mat4 ArticulatedModelNode::translation_q()
{
	return Angel::Translate(m_parent_joint_point);
}

Angel::vec3& ArticulatedModelNode::rotation_vec()
{
	return m_rotation;
}

/// <summary>
/// 
/// </summary>
/// <param name="parent_joint_height_normalized">
/// 0 stands for 
/// </param>
/// <param name="scale"></param>
/// <param name="rotation"></param>
/// <param name="texture"></param>
/// <param name="texture_slot"></param>
ArticulatedModelNode* ArticulatedModelNode::insert_child(
	const float parent_joint_height_normalized,
	const Angel::vec3& scale, 
	const Angel::vec3& rotation, 
	Texture* texture, 
	int texture_slot,
	unsigned int entity_id)
{
	auto* child = new ArticulatedModelNode(
		scale, 
		rotation, 
		texture, 
		texture_slot, 
		entity_id);

	ASSERT(parent_joint_height_normalized >= 0.0f && parent_joint_height_normalized <= 1.0f);
	child->m_parent_joint_point = (1 - parent_joint_height_normalized) * this->joint_point()
		+ (parent_joint_height_normalized) * (this->joint_point() + Angel::vec3(0, this->m_cube->scale().y, 0));

	child->m_parent = this;
	this->m_children_nodes.push_back(child);
	return child;
}

/// <summary>
/// Recursively generates the final transformation matrix by
/// looking up the parent matrices
/// </summary>
/// <returns></returns>
Angel::mat4 ArticulatedModelNode::model_matrix()
{
	if (m_parent != nullptr)
	{
		return m_parent->model_matrix()									// all parent transformations combined
			* translation_q() * rotation_u() * translation_minus_p();	// current body transformations
	}
	else
	{
		return rotation_u() * translation_minus_p();
	}
}

Angel::mat4 ArticulatedModelNode::cube_model_matrix()
{
	return m_cube->model_matrix();
}

const Texture* ArticulatedModelNode::cube_texture()
{
	return m_cube->texture();
}

int ArticulatedModelNode::cube_texture_slot()
{
	return m_cube->texture_slot();
}

const VertexArray* ArticulatedModelNode::cube_vao()
{
	return m_cube->vertex_array();
}

const IndexBuffer* ArticulatedModelNode::cube_ibo()
{
	return m_cube->index_buffer();
}

const Angel::vec3& ArticulatedModelNode::cube_scale()
{
	return m_cube->scale();
}

void ArticulatedModelNode::draw_node(
	const Angel::mat4& proj, 
	const Angel::mat4& view,
	const Angel::vec3& model_position)
{
	Angel::mat4 model_mat = Angel::Translate(model_position) * model_matrix() * m_cube->model_matrix();
	Angel::mat4 MV_matrix = view * model_mat;
	Angel::mat4 MVP_matrix = proj * view * model_mat;
	m_cube->texture()->bind(m_cube->texture_slot());
	Shape::textured_shader()->bind();
	Shape::textured_shader()->set_uniform_1i("u_texture", m_cube->texture_slot());
	Shape::textured_shader()->set_uniform_mat4f("u_MVP", MVP_matrix);
	Shape::textured_shader()->set_uniform_mat4f("u_MV", MV_matrix);
	Shape::textured_shader()->set_uniform_mat4f("u_P", proj);
	Angel::vec4 light_source_pos = view * Angel::vec4(0.0f, 1000.0f, 1000.0f, 1.0f);
	Shape::textured_shader()->set_uniform_4f("u_light_position", 
		light_source_pos.x, 
		light_source_pos.y, 
		light_source_pos.z, 
		light_source_pos.w);
	Shape::textured_shader()->set_uniform_4f("u_ambient", 0.32f, 0.173f, 0.118f, 1.0f);
	Shape::textured_shader()->set_uniform_4f("u_diffuse", 0.75f, 0.5f, 0.0f, 1.0f);
	Shape::textured_shader()->set_uniform_4f("u_specular", 1.0f, 1.0f, 1.0f, 1.0f);
	Shape::textured_shader()->set_uniform_1f("u_shininess", 50.0f);
	Shape::textured_shader()->set_uniform_1i("u_selected", m_is_selected);
	m_cube->texture()->bind(m_cube->texture_slot());
	Renderer::draw_triangles(m_cube->vertex_array(), m_cube->index_buffer(), Shape::textured_shader());
}

void ArticulatedModelNode::traverse_all(const std::function<void(ArticulatedModelNode*)>& f)
{
	f(this);
	for (auto* child : m_children_nodes)
	{
		if (child)
		{
			child->traverse_all(f);
		}
	}
}

void ArticulatedModelNode::destroy_children()
{
	for (auto child : m_children_nodes)
	{
		if (child != nullptr)
		{
			child->destroy_children();
			delete child;
			child = nullptr;
		}
	}
	m_children_nodes.clear();
}

Angel::vec3 ArticulatedModelNode::joint_point()
{
	return { 0.0f, -0.5f, 0.0f };
}

unsigned int random_uint(unsigned int min, unsigned int max)
{
	float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	return static_cast<unsigned int>(std::roundf(r * (max - min) + min));
}

float random_float(float min, float max)
{
	float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	return r * (max - min) + min;
}

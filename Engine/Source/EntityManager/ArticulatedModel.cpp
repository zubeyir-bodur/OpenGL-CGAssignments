#include "EntityManager/ArticulatedModel.h"
#include <functional>


ArticulatedModel::ArticulatedModel(
	const Angel::vec3& pos,
	Texture* texture,
	int texture_slot,
	const Angel::mat4& proj,
	const Angel::mat4& view)
{
	m_position = pos;
	m_texture = texture;
	texture_slot = m_texture_slot;
	m_num_nodes = 0;
	m_proj = const_cast<Angel::mat4*>(&proj);
	m_view = const_cast<Angel::mat4*>(&view);
	init_static_tree();
}

ArticulatedModel::~ArticulatedModel()
{
	destroy_tree();
}

void ArticulatedModel::init_random_tree(
	int branch_depth, 
	int min_children_per_trunk, 
	int max_children_per_trunk, 
	const Angel::vec3& initial_trunk_size)
{
	if (branch_depth > 0)
	{
		m_model_root = insert_child_to(nullptr, -1.0f,
			initial_trunk_size, { 0.0f, 0.0f, 0.0f }); // no rotation for the root trunk
		branch_depth--;
		unsigned int num_children = random_uint(min_children_per_trunk, max_children_per_trunk);
		for (unsigned int i = 0; (i < num_children && branch_depth > 0); i++)
		{
			Angel::vec3 rot;
			rot.x = random_float(-90.0f, 90.0f);
			rot.y = random_float(-90.0f, 90.0f);
			rot.z = random_float(-90.0f, 90.0f);
			ArticulatedModelNode* random_node = insert_child_to(m_model_root,
				1.0f, // first degree children of the root will always come out from the top of the root trunk
				initial_trunk_size / GOLDEN_RATIO,
				rot);
			init_random_tree(random_node, branch_depth - 1, min_children_per_trunk, max_children_per_trunk);
		}
	}
}

void ArticulatedModel::init_random_tree(
	ArticulatedModelNode* parent, 
	int branch_depth, 
	int min_children_per_trunk, 
	int max_children_per_trunk)
{
	if (branch_depth > 0)
	{
		unsigned int num_children = random_uint(min_children_per_trunk, max_children_per_trunk);
		for (unsigned int i = 0; i < num_children; i++)
		{
			Angel::vec3 rot;
			rot.x = random_float(-90.0f, 90.0f);
			rot.y = random_float(-90.0f, 90.0f);
			rot.z = random_float(-90.0f, 90.0f);
			float joint_pos_norm = random_float(0.0f, 1.0f);
			ArticulatedModelNode* random_node = insert_child_to(parent,
				joint_pos_norm,
				parent->cube_scale() / GOLDEN_RATIO,
				rot);
			init_random_tree(random_node, branch_depth - 1, min_children_per_trunk, max_children_per_trunk);
		}
	}
}

void ArticulatedModel::draw_model()
{
	Angel::mat4& proj = *m_proj;
	Angel::mat4& view = *m_view;
	if (m_model_root)
	{
		m_model_root->traverse_all([&proj, &view, tr_pos = m_position](ArticulatedModelNode* node) -> void
		{
			if (node)
			{
				node->draw_node(proj, view, tr_pos);
			}

		});
	}
}

void ArticulatedModel::traverse_all_nodes(const std::function<void(ArticulatedModelNode*)>& function)
{
	if (m_model_root)
	{
		m_model_root->traverse_all(function);
	}
}

ArticulatedModelNode* ArticulatedModel::get_node(unsigned int entity_id)
{
	ArticulatedModelNode* selected_node = nullptr;
	if (m_model_root)
	{
		m_model_root->traverse_all([&selected_node, entity_id](ArticulatedModelNode* node) -> void
		{
			if (node && node->entity_id() == entity_id)
			{
				selected_node = node;
				return;
			}
		});
	}
	return selected_node;
}

std::vector<Angel::vec3*> ArticulatedModel::collect_rotations()
{
	std::vector<Angel::vec3*> rotations;
	m_model_root->traverse_all([&rotations](ArticulatedModelNode* node) -> void
	{
		rotations.push_back(&node->rotation_vec());
	});
	return rotations;
}

/// <summary>
/// Initializes a predefined tree
/// </summary>
void ArticulatedModel::init_static_tree()
{
	m_model_root = insert_child_to(
		nullptr, -1.0f,
		{ 20.0f, 300.0f, 20.0f },
		{ 0.0f, 0.0f, 0.0f });

	auto leaf_1 = insert_child_to(
		m_model_root,
		1.0f,
		{ 15.0f, 100.0f, 15.0f },
		{ -15.0f, -20.0f, -55.0f });

	auto leaf_2 = insert_child_to(
		leaf_1,
		0.5f,
		{ 8.0f, 50.0f, 8.0f },
		{ 30.0f, 0.0f, 30.0f });

	auto leaf_3 = insert_child_to(
		m_model_root,
		1.0f,
		{ 15.0f, 100.0f, 15.0f },
		{ -15.0f, 20.0f, 55.0f });

	auto leaf_4 = insert_child_to(
		leaf_3, 
		0.5f,
		{ 8.0f, 50.0f, 8.0f },
		{ -13.0f, 0.0f, -45.0f });
}

/// <summary>
/// Each time an item is inserted, num nodes is incremented and
/// entity id is set to 2^24 - m_num_nodes
/// e.g. if there are 3 body parts including torso
///	4th body part will have entity id 16,777,212;
///	with the torso having the largest, MAX_ENTITY_ID=16,777,215
///
///	num_nodes++;
///	m_model_root = new(..., entity_id=MAX_ENTITY_ID - m_num_nodes + 1) ;
///	num_nodes++;
///	m_model_root->insert_child(..., entity_id=MAX_ENTITY_ID - m_num_nodes + 1);
/// ...
/// </summary>
/// <param name="parent"></param>
/// <param name="joint_pos_normalized"></param>
/// <param name="scale"></param>
/// <param name="rotation"></param>
/// <returns></returns>
ArticulatedModelNode* ArticulatedModel::insert_child_to(ArticulatedModelNode* parent, 
	const float joint_pos_normalized, 
	const Angel::vec3& scale,
	const Angel::vec3& rotation)
{
	m_num_nodes++;
	if (parent != nullptr)
	{
		return parent->insert_child(
			joint_pos_normalized,
			scale,
			rotation,
			m_texture, m_texture_slot, max_entity_id() - m_num_nodes + 1);
	}
	else
	{
		return new ArticulatedModelNode(
			scale,
			rotation,
			m_texture, m_texture_slot,
			max_entity_id() - m_num_nodes + 1);
	}
}

void ArticulatedModel::destroy_tree()
{
	if (m_model_root != nullptr)
	{
		m_model_root->destroy_children();
		delete m_model_root;
		m_model_root = nullptr;
	}
}


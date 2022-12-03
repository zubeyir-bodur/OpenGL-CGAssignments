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
	init_random_tree();
}

ArticulatedModel::~ArticulatedModel()
{
	destroy_tree();
}

void ArticulatedModel::draw_model()
{
	Angel::mat4& proj = *m_proj;
	Angel::mat4& view = *m_view;
	m_torso->traverse_all([&proj, &view, tr_pos=m_position](ArticulatedModelNode* node) -> void
	{
		node->draw_node(proj, view, tr_pos);

	});
}

void ArticulatedModel::traverse_all_nodes(const std::function<void(ArticulatedModelNode*)>& function)
{
	m_torso->traverse_all(function);
}

ArticulatedModelNode* ArticulatedModel::get_node(unsigned int entity_id)
{
	ArticulatedModelNode* selected_node = nullptr;
	m_torso->traverse_all([&selected_node, entity_id](ArticulatedModelNode* node) -> void
	{
		if (node->entity_id() == entity_id)
		{
			selected_node = node;
			return;
		}
	});
	return selected_node;
}

std::vector<Angel::vec3*> ArticulatedModel::collect_rotations()
{
	std::vector<Angel::vec3*> rotations;
	m_torso->traverse_all([&rotations](ArticulatedModelNode* node) -> void
	{
		rotations.push_back(&node->rotation_vec());
	});
	return rotations;
}

/// <summary>
/// Initializes random tree
/// 
///	Each time an item is inserted, num nodes is incremented and
///	entity id is set to 2^24 - m_num_nodes
///	e.g. if there are 3 body parts including torso
///	4th body part will have entity id 16,777,212;
///	with the torso having the largest, MAX_ENTITY_ID=16,777,215
///	
///	num_nodes++;
///	m_torso = new(..., entity_id=MAX_ENTITY_ID - m_num_nodes + 1) ;
///	num_nodes++;
///	m_torso->insert_child(..., entity_id=MAX_ENTITY_ID - m_num_nodes + 1);
///	...
/// </summary>
void ArticulatedModel::init_random_tree()
{
	const unsigned int MAX_ENTITY_ID = max_entity_id();
	m_num_nodes++;
	m_torso = new ArticulatedModelNode(
		{ 20.0f, 300.0f, 20.0f },
		{0.0f, 0.0f, 0.0f },
		m_texture, m_texture_slot,
		MAX_ENTITY_ID - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_1 = m_torso->insert_child(
		1.0f, 
		{ 15.0f, 100.0f, 15.0f },
		{ -15.0f, -15.0f, -55.0f },
		m_texture, 
		m_texture_slot, 
		MAX_ENTITY_ID - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_2 = leaf_1->insert_child(
		0.5f, 
		{ 8.0f, 50.0f, 8.0f },
		{ -16.0f, -120.0f, -66.0f },
		m_texture, 
		m_texture_slot, 
		MAX_ENTITY_ID - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_3 = m_torso->insert_child(
		1.0f, 
		{ 15.0f, 100.0f, 15.0f },
		{ 45.0f, -20.0f, 55.0f },
		m_texture, m_texture_slot, MAX_ENTITY_ID - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_4 = leaf_3->insert_child(
		0.5f, 
		{ 8.0f, 50.0f, 8.0f },
		{ -140.0f, -65.0f, -90.0f },
		m_texture, m_texture_slot, MAX_ENTITY_ID - m_num_nodes + 1);
}

void ArticulatedModel::destroy_tree()
{
	if (m_torso != nullptr)
	{
		m_torso->destroy_children();
		delete m_torso;
	}
}

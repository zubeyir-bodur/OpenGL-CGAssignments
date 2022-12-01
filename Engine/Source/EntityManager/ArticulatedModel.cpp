#include "EntityManager/ArticulatedModel.h"
#include <functional>

ArticulatedModel::ArticulatedModel(
	const Angel::vec3& pos,
	Texture* texture,
	int texture_slot)
{
	m_position = pos;
	m_texture = texture;
	texture_slot = m_texture_slot;
	m_num_nodes = 0;
	init_random_tree();
}

ArticulatedModel::~ArticulatedModel()
{
	destroy_tree();
}

void ArticulatedModel::draw_model(
	const Angel::mat4& proj, 
	const Angel::mat4& view)
{
	m_torso->traverse_all([&proj, &view, tr_pos=m_position](ArticulatedModelNode* node) -> void
	{
		node->draw_node(proj, view, tr_pos);

	});
}

std::vector<Angel::vec3*> ArticulatedModel::collect_rotations()
{
	std::vector<Angel::vec3*> rotations;
	m_torso->traverse_all([&rotations](ArticulatedModelNode* node) -> void
	{
		rotations.push_back(&node->m_rotation);
	});
	return rotations;
}

Angel::vec3& ArticulatedModel::position()
{
	return m_position;
}

/// <summary>
/// Initializes random tree
/// </summary>
void ArticulatedModel::init_random_tree()
{
	// TODO
	// 
	// Each time an item is inserted, num nodes is incremented and
	// entity id is set to UINT_MAX - m_num_nodes + 1
	// e.g. if there are 3 body parts including torso
	// 4th body part will have entity id 4,294,967,292;
	// with the torso having the largest, 4,294,967,295
	// 
	// num_nodes++;
	// m_torso = new(..., entity_id=UINT_MAX - m_num_nodes + 1) ;
	// num_nodes++;
	// m_torso->insert_child(..., entity_id=UINT_MAX - m_num_nodes + 1);
	// ...
}

void ArticulatedModel::destroy_tree()
{
	if (m_torso != nullptr)
	{
		m_torso->destroy_children();
		delete m_torso;
	}
}

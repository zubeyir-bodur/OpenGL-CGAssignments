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
	m_num_nodes++;
	m_torso = new ArticulatedModelNode(Angel::vec3(20, 300, 20), {0, 0, 0},
		m_texture, m_texture_slot,
		UINT_MAX - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_1 = m_torso->insert_child(1.0f, { 15, 100, 15 }, { -45, 45, -45 }, m_texture, m_texture_slot, UINT_MAX - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_2 = leaf_1->insert_child(1.0f, { 8, 50, 8 }, { 45, 10, 10 }, m_texture, m_texture_slot, UINT_MAX - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_3 = m_torso->insert_child(1.0f, { 15, 100, 15 }, { 45, 45, 45 }, m_texture, m_texture_slot, UINT_MAX - m_num_nodes + 1);

	m_num_nodes++;
	auto leaf_4 = leaf_3->insert_child(0.5f, { 8, 50, 8 }, { -90, -45, -90 }, m_texture, m_texture_slot, UINT_MAX - m_num_nodes + 1);
}

void ArticulatedModel::destroy_tree()
{
	if (m_torso != nullptr)
	{
		m_torso->destroy_children();
		delete m_torso;
	}
}

#pragma once
#include "EntityManager/ArticulatedModelNode.h"

class ArticulatedModel
{
private:
	ArticulatedModelNode* m_torso;
	Angel::vec3 m_position;
	Angel::mat4 *m_proj, *m_view;
	Texture* m_texture;
	int m_texture_slot;
	unsigned int m_num_nodes;

	void init_random_tree();
	void destroy_tree();
public:
	ArticulatedModel(
		const Angel::vec3& pos,
		Texture* texture,
		int texture_slot,
		const Angel::mat4& proj,
		const Angel::mat4& view);

	~ArticulatedModel();
	void draw_model();
	void traverse_all_nodes(const std::function<void(ArticulatedModelNode*)>& function);
	ArticulatedModelNode* get_node(unsigned int entity_id);
	std::vector<Angel::vec3*> collect_rotations();
	inline Angel::vec3& position() { return m_position; }
	inline const ArticulatedModelNode* torso() { return m_torso; }
	inline const unsigned int& num_nodes() { return m_num_nodes; }
	static inline const unsigned int max_entity_id() { return pow(2, 24) - 1; }
	inline const unsigned int min_entity_id() { return pow(2, 24) - m_num_nodes; }
};

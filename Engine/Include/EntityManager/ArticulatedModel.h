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
	std::vector<Angel::vec3*> collect_rotations();
	inline Angel::vec3& position();
};
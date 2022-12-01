#pragma once
#include "EntityManager/ArticulatedModelNode.h"

class ArticulatedModel
{
private:
	ArticulatedModelNode* m_torso;
	Angel::vec3 m_position;
	Texture* m_texture;
	int m_texture_slot;
	unsigned int m_num_nodes;

	void init_random_tree();
	void destroy_tree();
public:
	ArticulatedModel(
		const Angel::vec3& pos,
		Texture* texture,
		int texture_slot);
	~ArticulatedModel();
	void draw_model(
		const Angel::mat4& proj,
		const Angel::mat4& view);
	std::vector<Angel::vec3*> collect_rotations();
	inline Angel::vec3& position();
};

#pragma once
#include "EntityManager/ArticulatedModelNode.h"

class ArticulatedModel
{
private:
	ArticulatedModelNode* m_model_root;
	Angel::vec3 m_position;
	Angel::mat4 *m_proj, *m_view;
	Texture* m_texture;
	int m_texture_slot;
	unsigned int m_num_nodes;

	void init_static_tree();
	ArticulatedModelNode* insert_child_to(ArticulatedModelNode* parent,
		const float joint_pos_normalized,
		const Angel::vec3& scale,
		const Angel::vec3& rotation
	);

	/// <summary>
	/// Initializes a random tree beneath parent with branch_depth # of height
	/// </summary>
	/// <param name="branch_depth"></param>
	/// <param name="min_children_per_trunk"></param>
	/// <param name="max_children_per_trunk"></param>
	void init_random_tree(
		ArticulatedModelNode* parent,
		int branch_depth,
		int min_children_per_trunk,
		int max_children_per_trunk);

public:
	ArticulatedModel(
		const Angel::vec3& pos,
		Texture* texture,
		int texture_slot,
		const Angel::mat4& proj,
		const Angel::mat4& view);

	~ArticulatedModel();
	void init_random_tree(int branch_depth,
		int min_children_per_trunk,
		int max_children_per_trunk,
		const Angel::vec3& initial_trunk_size);
	void destroy_tree();
	void draw_model();
	void traverse_all_nodes(const std::function<void(ArticulatedModelNode*)>& function);
	ArticulatedModelNode* get_node(unsigned int entity_id);
	std::vector<Angel::vec3*> collect_rotations();
	inline Angel::vec3& position() { return m_position; }
	inline const ArticulatedModelNode* torso() { return m_model_root; }
	inline const unsigned int& num_nodes() { return m_num_nodes; }
	static inline const unsigned int max_entity_id() { return static_cast<unsigned int>(pow(2, 24)) - 1; }
	inline const unsigned int min_entity_id() { return static_cast<unsigned int>(pow(2, 24)) - m_num_nodes; }
};

constexpr float GOLDEN_RATIO = 1.61803398875f;

unsigned int random_uint(unsigned int min, unsigned int max);
float random_float(float min, float max);
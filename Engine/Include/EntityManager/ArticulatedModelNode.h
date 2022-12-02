#pragma once
#include "EntityManager/ShapeModel.h"

// forward declare function
namespace std {
	template <class T>
	class function;
}

class ArticulatedModelNode
{
private:
	ShapeModel* m_cube;
	ArticulatedModelNode* m_parent;
	std::vector<ArticulatedModelNode*> m_children_nodes;
	/// <summary>
	/// q of its parent
	/// </summary>
	Angel::vec3 m_parent_joint_point;
	unsigned int m_entity_id;

	/// <summary>
	/// p = (0, -0.5, 0), every branch will rotate around its bottom
	/// </summary>
	/// <returns></returns>
	Angel::vec3 joint_point();	
public:
	/// <summary>
	/// u
	/// </summary>
	Angel::vec3 m_rotation;

	ArticulatedModelNode(
		const Angel::vec3& scale,
		const Angel::vec3& rotation,
		Texture* texture,
		int texture_slot,
		unsigned int entity_id);
	~ArticulatedModelNode();

	Angel::mat4 rotation_u();
	Angel::mat4 translation_minus_p();
	Angel::mat4 translation_q();

	ArticulatedModelNode* insert_child(
		const float parent_joint_height_normalized,
		const Angel::vec3& scale, 
		const Angel::vec3& rotation,
		Texture* texture,
		int texture_slot,
		unsigned int entity_id);
	Angel::mat4 model_matrix();
	void draw_node(
		const Angel::mat4& proj,
		const Angel::mat4& view,
		const Angel::vec3& model_position
	);
	void traverse_all(const std::function<void(ArticulatedModelNode*)>& f);
	void destroy_children();
};
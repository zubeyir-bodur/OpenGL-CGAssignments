#pragma once
#include "EntityManager/Shape.h"
#include "Renderer/Texture.h"

class ShapeModel
{
public:
	enum class StaticShape
	{
		NONE,
		RECTANGLE,
		ISOSCELES_TRIANGLE,
		COL_CUBE,
		TEX_CUBE,
	};
private:
	Shape* m_shape_def;
	StaticShape m_e_def;
	bool m_is_poly = false;
	bool m_is_selected = false;
	bool m_is_hidden = false;
	int m_texture_slot = -1;
	Texture* m_texture = nullptr;

	// Model dependent members
	Angel::vec3* m_position; // middle point of the geometric shape
	Angel::vec3* m_rotation; // in angles
	Angel::vec3* m_scale;	 // scaling from the middle point
	Angel::vec4* m_color;
public:
	// For predefined unit colored shapes
	// Colored cube is also supported here
	ShapeModel(StaticShape def,
		Angel::vec3* pos,
		Angel::vec3* rot,
		Angel::vec3* scale,
		Angel::vec4* rgba = nullptr);

	// For convex polygons with varying vertex numbers
	ShapeModel(const std::vector<Angel::vec3>& poly_mouse_model_coords,
		Angel::vec4* rgba);

	// For textured shapes, currently only cubes support textures
	ShapeModel(StaticShape def,
		Angel::vec3* pos,
		Angel::vec3* rot,
		Angel::vec3* scale,
		int texture_slot,
		Texture* texture);

	~ShapeModel();

	inline Angel::vec3& position() { return *m_position; }
	inline Angel::vec3& rotation() { return *m_rotation; }
	inline Angel::vec3& scale() { return *m_scale; }
	inline Angel::vec4& color() { return *m_color; }
	inline bool& is_hidden() { return m_is_hidden; }
	inline Texture* texture() { return m_texture; }
	inline int texture_slot() { return m_texture_slot; }
	inline const VertexArray* vertex_array() { return m_shape_def->vertex_array(); }
	inline const IndexBuffer* index_buffer() { return m_shape_def->index_buffer(); }
	inline void select() { m_is_selected = true; }
	inline void deselect() { m_is_selected = false; }
	inline bool is_selected() { return m_is_selected; }
	inline StaticShape shape_def() { return m_e_def; }
	inline const float is_poly() { return m_is_poly; }
	inline std::vector<float> raw_vertices() { return m_shape_def->vertices(); }

	bool contains_2d(const Angel::vec3& model_pos);
	unsigned int true_num_vertices();
	std::vector<Angel::vec3> model_coords();
	Angel::mat4 model_matrix();
	void push_back_vertex(const Angel::vec3& mouse_model_pos);
	Angel::vec3 center_raw();
	Angel::vec3 center_raw_bottom();
	Angel::vec3 center_true();
	std::array<float, 6> shape_bounding_cube();
	Angel::vec3 shape_size();
	void draw_shape(const Angel::mat4& proj, const Angel::mat4& view);

	static std::array<float, 6> bounding_cube(const std::vector<ShapeModel*>& shapes);
};
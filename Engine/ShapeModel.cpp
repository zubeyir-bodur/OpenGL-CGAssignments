#include "ShapeModel.h"
#include "ErrorManager.h"

ShapeModel::ShapeModel(StaticShape def,
	Angel::vec3* pos,
	Angel::vec3* rot,
	Angel::vec3* scale,
	Angel::vec4* rgba)
{
	switch (def)
	{
	case ShapeModel::StaticShape::RECTANGLE:
		m_shape_def = const_cast<Shape*>(Shape::rectangle());
		break;
	case ShapeModel::StaticShape::EQUILATERAL_TRIANGLE:
		m_shape_def = const_cast<Shape*>(Shape::eq_triangle());
		break;
	default:
		ASSERT(false && "This predefined shape is not implemented!");
		break;
	}
	m_is_poly = false;
	m_position = pos;
	m_rotation = rot;
	m_scale = scale;
	m_color = rgba;
}

ShapeModel::ShapeModel(const std::vector<Angel::vec3>& poly_coords,
	Angel::vec3* pos,
	Angel::vec3* rot,
	Angel::vec3* scale,
	Angel::vec4* rgba)
{
	m_is_poly = true;
	m_shape_def = new Shape(poly_coords);
	m_position = pos;
	m_rotation = rot;
	m_scale = scale;
	m_color = rgba;
}

ShapeModel::~ShapeModel()
{
	if (m_is_poly)
	{
		delete m_shape_def;
	}
}


bool ShapeModel::contains(const Angel::vec3& model_pos)
{
	unsigned int nvert = m_shape_def->num_vertices();
	auto model_coordinates = model_coords();
	unsigned int i, j;
	bool c = false;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		float x_i = model_coordinates[i].x;
		float y_i = model_coordinates[i].y;
		float x_j = model_coordinates[j].x;
		float y_j = model_coordinates[j].y;
		if (
			((y_i > model_pos.y) != (y_j > model_pos.y)) &&
			(model_pos.x < (x_j - x_i) * (model_pos.y - y_i) / (y_j - y_i) + x_j)
			)
			c = !c;
	}
	return c;
}

std::vector<Angel::vec3> ShapeModel::model_coords()
{
	const std::vector<float>& raw_vertices = m_shape_def->vertices();
	Angel::mat4 mat_model = model_matrix();
	std::vector<Angel::vec3> out(m_shape_def->num_vertices());
	int idx_vertex = 0;
	for (auto& vertex : out)
	{
		float x = raw_vertices[idx_vertex * NUM_COORDINATES];
		float y = raw_vertices[idx_vertex * NUM_COORDINATES + 1];
		float z = raw_vertices[idx_vertex * NUM_COORDINATES + 2];
		Angel::vec4 tmp = mat_model * Angel::vec4(x, y, z, 1.0f);
		vertex.x = tmp.x;
		vertex.y = tmp.y;
		vertex.z = tmp.z;
		idx_vertex++;
	}
	return out;
}

Angel::mat4 ShapeModel::model_matrix()
{
	return Angel::Translate(center_position())
		//* Angel::rotate(Angel::mat4(1.0f), Angel::radians((*m_rotation).x), Angel::vec3(1, 0, 0))
		//* Angel::rotate(Angel::mat4(1.0f), Angel::radians((*m_rotation).y), Angel::vec3(0, 1, 0))
		* Angel::RotateZ(((*m_rotation).z)) // required rotation for assignment 1
		* Angel::Translate(-center_position() + (*m_position));
		* Angel::Scale(*m_scale);
}

void ShapeModel::push_back_vertex(const Angel::vec3& model_pos)
{
	m_shape_def->push_back_vertex(model_pos);
}

Angel::vec3 ShapeModel::center_position()
{
	Angel::vec3 center(0.0f, 0.0f, 0.0f);
	std::vector<float> vert = m_shape_def->vertices();
	for (unsigned int i = 0; i < m_shape_def->num_vertices() * NUM_COORDINATES; i+= NUM_COORDINATES)
	{
		center.x += (vert[i]     - (*m_position).x) * (*m_scale).x + 2 * (*m_position).x;
		center.y += (vert[i + 1] - (*m_position).y) * (*m_scale).y + 2 * (*m_position).y;
		center.z += (vert[i + 2] - (*m_position).z) * (*m_scale).z + 2 * (*m_position).z;
	}
	center /= (float)m_shape_def->num_vertices();
	return center;
}

std::array<float, 6> ShapeModel::shape_bounding_cube()
{
	float x_min = (float)INT_MAX, x_max = (float)INT_MIN, y_min = (float)INT_MAX, y_max = (float)INT_MIN, z_min = (float)INT_MAX, z_max = (float)INT_MIN;
	auto model_coordinates = model_coords();
	if (model_coordinates.empty())
	{
		return { 0, 0, 0, 0, 0, 0 };
	}
	for (auto& vert_coord : model_coordinates)
	{
		if (vert_coord.x < x_min)
		{
			x_min = vert_coord.x;
		}
		if (vert_coord.x > x_max)
		{
			x_max = vert_coord.x;
		}
		if (vert_coord.y < y_min)
		{
			y_min = vert_coord.y;
		}
		if (vert_coord.y > y_max)
		{
			y_max = vert_coord.y;
		}
		if (vert_coord.z < z_min)
		{
			z_min = vert_coord.z;
		}
		if (vert_coord.z > z_max)
		{
			z_max = vert_coord.z;
		}
	}
	return {
		x_min, x_max, y_min, y_max, z_min, z_max
	};
}

Angel::vec3 ShapeModel::shape_size()
{
	std::array<float, 6> bounding_cube = this->shape_bounding_cube();
	return {
		(bounding_cube[1] - bounding_cube[0]) * (*m_scale).x,
		(bounding_cube[3] - bounding_cube[2]) * (*m_scale).y,
		(bounding_cube[5] - bounding_cube[4]) * (*m_scale).z
	};
}

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
	case ShapeModel::StaticShape::ISOSCELES_TRIANGLE:
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
	m_e_def = def;
}

ShapeModel::ShapeModel(const std::vector<Angel::vec3>& poly_mouse_model_coords,
	Angel::vec4* rgba)
{
	m_is_poly = true;

	m_position = new Angel::vec3(0.0f, 0.0f, 0.0f);
	m_rotation = new Angel::vec3(0.0f, 0.0f, 0.0f);
	m_scale = new Angel::vec3(1.0f, 1.0f, 1.0f);

	for (unsigned int i = 0; i < poly_mouse_model_coords.size(); i++)
	{
		*m_position += poly_mouse_model_coords[i];
	}
	*m_position /= (float)poly_mouse_model_coords.size();


	std::vector<Angel::vec3> poly_mouse_model_coords_minus_center;
	poly_mouse_model_coords_minus_center.reserve(poly_mouse_model_coords.size());
	for (unsigned int i = 0; i < poly_mouse_model_coords.size(); i++)
	{
		poly_mouse_model_coords_minus_center.emplace_back(poly_mouse_model_coords[i] - (*m_position));
	}
	m_shape_def = new Shape(poly_mouse_model_coords_minus_center);
	m_color = rgba;
	m_e_def = StaticShape::NONE;
}

ShapeModel::~ShapeModel()
{
	if (m_is_poly)
	{
		delete m_shape_def;
	}
	delete m_position;
	delete m_rotation;
	delete m_scale;
	delete m_color;
}

// Polygon test function, taken from :
// https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon
// The code was changed in the following way:
//  - Functions were converted into lambdas
//  - Since the point inclusion in the code was for integers, it was converted to floats
//  - The code in the link works for statically allocated arrays of Points, which is not compatible
//       with the API. It was modified so that it works with the Angel::vec3 data type
//  - Some of the names of the variables were changed so that code is cleaner
bool ShapeModel::contains(const Angel::vec3& model_pos)
{
	struct Point {
		float x, y;
	};

	struct line {
		Point p1, p2;
	};

	auto on_line = [](line l1, Point p) -> bool
	{
		// Check whether p is on the line or not
		if (p.x <= std::max(l1.p1.x, l1.p2.x)
			&& p.x <= std::min(l1.p1.x, l1.p2.x)
			&& (p.y <= std::max(l1.p1.y, l1.p2.y)
				&& p.y <= std::min(l1.p1.y, l1.p2.y)))
			return true;

		return false;
	};

	auto direction = [](Point a, Point b, Point c) -> int
	{
		float val = (b.y - a.y) * (c.x - b.x)
			- (b.x - a.x) * (c.y - b.y);
		if (val == 0)
			// Colinear
			return 0;
		else if (val < 0)
			// Anti-clockwise direction
			return 2;
		// Clockwise direction
		return 1;
	};

	auto is_intersect = [&, direction, on_line](line l1, line l2) -> bool
	{
		// Four direction for two lines and points of other line
		int dir1 = direction(l1.p1, l1.p2, l2.p1);
		int dir2 = direction(l1.p1, l1.p2, l2.p2);
		int dir3 = direction(l2.p1, l2.p2, l1.p1);
		int dir4 = direction(l2.p1, l2.p2, l1.p2);

		// When intersecting
		if (dir1 != dir2 && dir3 != dir4)
			return true;

		// When p2 of line2 are on the line1
		if (dir1 == 0 && on_line(l1, l2.p1))
			return true;

		// When p1 of line2 are on the line1
		if (dir2 == 0 && on_line(l1, l2.p2))
			return true;

		// When p2 of line1 are on the line2
		if (dir3 == 0 && on_line(l2, l1.p1))
			return true;

		// When p1 of line1 are on the line2
		if (dir4 == 0 && on_line(l2, l1.p2))
			return true;

		return false;
	};

	auto model_coordinates = model_coords();
	unsigned int num_vertices = model_coordinates.size();

	// When polygon has less than 3 edge, it is not polygon
	if (num_vertices < 3)
		return false;
	Point p = { model_pos.x, model_pos.y };
	// Create a point at infinity, y is same as point p
	line exline = { p, { INT_MAX, p.y } };
	int count = 0;
	int i = 0;
	do {

		// Forming a line from two consecutive points of our shape
		Point p1 = { model_coordinates[i].x, model_coordinates[i].y };
		Point p2 = { model_coordinates[(i + 1) % num_vertices].x, model_coordinates[(i + 1) % num_vertices].y };
		line side = { p1, p2 };
		if (is_intersect(side, exline)) {

			// If exline is on top of this side
			if (direction(side.p1, p, side.p2) == 0)
					return on_line(side, p);
			count++;
		}
		i = (i + 1) % num_vertices;
	} while (i != 0);

	// When count is odd
	return count & 1;
}

unsigned int ShapeModel::true_num_vertices()
{
	unsigned int n_vert = m_shape_def->num_vertices();
	if (m_e_def == StaticShape::NONE)
	{
		n_vert -= 1;
	}
	return n_vert;
}

std::vector<Angel::vec3> ShapeModel::model_coords()
{
	std::vector<float> raw_vertices = m_shape_def->vertices();
	if (m_e_def == StaticShape::NONE)
	{
		// Exclude the first vertex, which is the precomputed mid point
		raw_vertices.erase(raw_vertices.begin(), raw_vertices.begin() + NUM_COORDINATES);
	}
	Angel::mat4 mat_model = model_matrix();
	std::vector<Angel::vec3> out;
	if (m_e_def == StaticShape::NONE)
	{
		out.reserve(m_shape_def->num_vertices() - 1);
	}
	else
	{
		out.reserve(m_shape_def->num_vertices());
	}
	for (unsigned int i = 0; i < out.capacity(); i++)
	{
		float x = raw_vertices[i * NUM_COORDINATES];
		float y = raw_vertices[i * NUM_COORDINATES + 1];
		float z = raw_vertices[i * NUM_COORDINATES + 2];
		Angel::vec4 tmp = mat_model * Angel::vec4(x, y, z, 1.0f);
		out.emplace_back(tmp.x, tmp.y, tmp.z);
	}
	return out;
}

Angel::mat4 ShapeModel::model_matrix()
{
	return ((Angel::Translate((*m_position))
		//* Angel::rotate(Angel::mat4(1.0f), Angel::radians((*m_rotation).x), Angel::vec3(1, 0, 0)) not necessary  for assignment 1
		//* Angel::rotate(Angel::mat4(1.0f), Angel::radians((*m_rotation).y), Angel::vec3(0, 1, 0)) not necessary  for assignment 1
		* Angel::RotateZ(((*m_rotation).z))) // required rotation for assignment 1
		* Angel::Scale(*m_scale))
		* Angel::Translate(-center_raw());
}

void ShapeModel::push_back_vertex(const Angel::vec3& model_pos)
{
	*m_position = m_shape_def->push_back_vertex(model_pos - (*m_position), (*m_position));
}

Angel::vec3 ShapeModel::center_raw()
{
	Angel::vec3 center(0.0f, 0.0f, 0.0f);
	std::vector<float> vert = m_shape_def->vertices();
	for (unsigned int i = 0; i < m_shape_def->num_vertices() * NUM_COORDINATES; i+= NUM_COORDINATES)
	{
		center.x += vert[i]    ;
		center.y += vert[i + 1];
		center.z += vert[i + 2];
	}
	if (m_e_def == StaticShape::NONE)
	{
		center /= (float)m_shape_def->num_vertices() - 1;
	}
	else
	{
		center /= (float)m_shape_def->num_vertices();
	}
	return center;
}

Angel::vec3 ShapeModel::center_true()
{
	return center_raw() + *m_position;
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

std::array<float, 6> ShapeModel::bounding_cube(const std::vector<ShapeModel*>& shapes)
{
	std::array<float, 6> out_bounding_cube = {INT_MAX, INT_MIN, INT_MAX, INT_MIN, INT_MAX, INT_MIN};
	for (unsigned int i = 0; i < shapes.size(); i++)
	{
		std::array<float, 6> cur_bounding_cube = shapes[i]->shape_bounding_cube();
		for (unsigned int j = 0; j < out_bounding_cube.size(); j += 2)
		{
			if (cur_bounding_cube[j] < out_bounding_cube[j])
			{
				out_bounding_cube[j] = cur_bounding_cube[j];
			}
		}
		for (unsigned int j = 1; j < out_bounding_cube.size(); j += 2)
		{
			if (cur_bounding_cube[j] > out_bounding_cube[j])
			{
				out_bounding_cube[j] = cur_bounding_cube[j];
			}
		}
	}
	return out_bounding_cube;
}


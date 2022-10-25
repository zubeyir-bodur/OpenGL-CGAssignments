#include "DSerializer.h"
#include "ErrorManager.h"
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include <optional>

/// <summary>
/// The format is the following:
/// 
/// Example RECTANGLE shape serialized:
/// 
/// ShapeModel
///		RECTANGLE
///		pos.x pos.y pos.z
///		rot.x rot.y rot.z
///		scale.x scale.y scale.z
///		col.r col.g col.b col.a
///		
///	Example POLYGON shape serialized:
///	
///	ShapeModel
///		NONE
///		BEGIN
///		vertices[0].x vertices[0].y vertices[0].z
///		...
///		vertices[n-1].x vertices[n-1].y vertices[n-1].z
///		END
///		rot.x rot.y rot.z
///		col.r col.g col.b col.a
///		
///	For polygons, scale was assumed to be 1, 1, 1 for all.
/// </summary>
/// <param name="drawlist"></param>
/// <param name="serialize_path"></param>
void DSerializer::serialize_drawlist(const std::vector<ShapeModel*>& drawlist, const std::filesystem::path& serialize_path)
{
	std::filesystem::create_directories(serialize_path.parent_path());
	std::ofstream file(serialize_path);
	for (auto& shape : drawlist)
	{
		// Do not serialize undone shapes
		if (!shape->is_hidden())
		{
			file << "ShapeModel" << std::endl;
			ShapeModel::StaticShape type = shape->shape_def();
			file << "\t" << magic_enum::enum_name(type).data() << std::endl;
			if (type != ShapeModel::StaticShape::NONE)
			{
				Angel::vec3 pos = shape->position();
				Angel::vec3 rot = shape->rotation();
				Angel::vec3 scale = shape->scale();
				Angel::vec4 col = shape->color();
				file << "\t" << std::to_string(pos.x) << " " << std::to_string(pos.y) << " " << std::to_string(pos.z) << std::endl;
				file << "\t" << std::to_string(rot.x) << " " << std::to_string(rot.y) << " " << std::to_string(rot.z) << std::endl;
				file << "\t" << std::to_string(scale.x) << " " << std::to_string(scale.y) << " " << std::to_string(scale.z) << std::endl;
				file << "\t" << std::to_string(col.x) << " " << std::to_string(col.y) << " " << std::to_string(col.z) << " " << std::to_string(col.w) << std::endl;
			}
			else
			{
				std::vector<float> translation_only_coords = shape->raw_vertices();
				// ignore the 0th(center) and last(1st repeated) vertices
				translation_only_coords.erase(translation_only_coords.begin(), translation_only_coords.begin() + NUM_COORDINATES);
				translation_only_coords.erase(translation_only_coords.end() - NUM_COORDINATES, translation_only_coords.end());
				Angel::vec3 pos = shape->position();
				Angel::vec3 rot = shape->rotation();
				Angel::vec4 col = shape->color();
				file << "\tBEGIN" << std::endl;
				for (unsigned int i = 0; i < translation_only_coords.size(); i+=NUM_COORDINATES)
				{
					translation_only_coords[i] += pos.x;
					translation_only_coords[i+1] += pos.y;
					translation_only_coords[i+2] += pos.z;
					file << "\t" << std::to_string(translation_only_coords[i]) << " " << std::to_string(translation_only_coords[i+1]) << " " << std::to_string(translation_only_coords[i+2]) << std::endl;
				}
				file << "\tEND" << std::endl;
				file << "\t" << std::to_string(rot.x) << " " << std::to_string(rot.y) << " " << std::to_string(rot.z) << std::endl;
				file << "\t" << std::to_string(col.x) << " " << std::to_string(col.y) << " " << std::to_string(col.z) << " " << std::to_string(col.w) << std::endl;
			}
		}
	}
	file.close();
}

std::vector<ShapeModel*> DSerializer::deserialize_drawlist(const std::filesystem::path& file_path)
{
	auto ltrim_then_split = [](std::string line) -> std::vector<float>
	{
		line.erase(0, line.find_first_not_of("\t"));
		std::string delimiter = " ";
		std::vector<float> split;
		std::string token;
		size_t pos = 0;
		while ((pos = line.find(delimiter)) != std::string::npos) {
			token = line.substr(0, pos);
			split.push_back(std::stof(token));
			line.erase(0, pos + delimiter.length());
		}
		split.push_back(std::stof(line));
		return split;
	};

	// Output state
	std::ifstream stream(file_path.string());
	std::string line;
	std::stringstream ss;
	std::vector<ShapeModel*> out;

	// Enum names to string
	std::string rect_name = magic_enum::enum_name(ShapeModel::StaticShape::RECTANGLE).data();
	std::string tri_name = magic_enum::enum_name(ShapeModel::StaticShape::ISOSCELES_TRIANGLE).data();
	std::string poly_name = magic_enum::enum_name(ShapeModel::StaticShape::NONE).data();

	// Current shape state
	unsigned int line_idx_for_cur_shape = -1;
	ShapeModel::StaticShape type;
	Angel::vec3* cur_pos = nullptr;
	Angel::vec3* cur_rot = nullptr;
	Angel::vec3* cur_scale = nullptr;
	Angel::vec4* cur_col = nullptr;
	std::vector<Angel::vec3> cur_model_coords;

	while (getline(stream, line))
	{
		if (line == "ShapeModel" && line_idx_for_cur_shape == -1)
		{
			line_idx_for_cur_shape = 0;
			cur_pos = nullptr;
			cur_rot = nullptr;
			cur_scale = nullptr;
			cur_col = nullptr;
			cur_model_coords = {};
			getline(stream, line);
			line_idx_for_cur_shape++;
			if (line == "\t" + rect_name)
			{
				type = magic_enum::enum_cast<ShapeModel::StaticShape>(rect_name).value();
			}
			else if (line == "\t" + tri_name)
			{
				type = magic_enum::enum_cast<ShapeModel::StaticShape>(tri_name).value();
			}
			else if (line == "\t" + poly_name)
			{
				type = magic_enum::enum_cast<ShapeModel::StaticShape>(poly_name).value();
			}
			if (type != ShapeModel::StaticShape::NONE)
			{
				// Process pos
				getline(stream, line);
				line_idx_for_cur_shape++;
				std::vector<float> positions = ltrim_then_split(line);
				ASSERT(positions.size() >= NUM_COORDINATES);
				cur_pos = new Angel::vec3{ positions[0], positions[1], positions[2] };
				if (positions.size() > NUM_COORDINATES)
				{
					std::cout << "Warning, loaded ShapeModel position had more than " << NUM_COORDINATES << " coordinates!" << std::endl;
				}
				line_idx_for_cur_shape++;

				// Process rot
				getline(stream, line);
				line_idx_for_cur_shape++;
				std::vector<float> rotations = ltrim_then_split(line);
				ASSERT(rotations.size() >= NUM_COORDINATES);
				cur_rot = new Angel::vec3{ rotations[0], rotations[1], rotations[2] };
				if (rotations.size() > NUM_COORDINATES)
				{
					std::cout << "Warning, loaded ShapeModel rotation had more than " << NUM_COORDINATES << " coordinates!" << std::endl;
				}
				line_idx_for_cur_shape++;

				// Process scale
				getline(stream, line);
				line_idx_for_cur_shape++;
				std::vector<float> scales = ltrim_then_split(line);
				ASSERT(scales.size() >= NUM_COORDINATES);
				cur_scale = new Angel::vec3{ scales[0], scales[1], scales[2] };
				if (scales.size() > NUM_COORDINATES)
				{
					std::cout << "Warning, loaded ShapeModel scales had more than " << NUM_COORDINATES << " coordinates!" << std::endl;
				}

				// Process col
				getline(stream, line);
				line_idx_for_cur_shape++;
				std::vector<float> colors = ltrim_then_split(line);
				ASSERT(colors.size() >= 4);
				cur_col = new Angel::vec4{ colors[0], colors[1], colors[2], colors[3] };
				if (colors.size() > 4)
				{
					std::cout << "Warning, loaded ShapeModel colors had more than 4 values!" << std::endl;
				}

				// Create Shape
				ShapeModel* cur = new ShapeModel(type, cur_pos, cur_rot, cur_scale, cur_col);
				out.push_back(cur);
				line_idx_for_cur_shape = -1;
			}
			else if (!line.empty())
			{
				getline(stream, line);
				line_idx_for_cur_shape++;
				ASSERT(line == "\tBEGIN");
				getline(stream, line);
				line_idx_for_cur_shape++;
				do 
				{
					std::vector<float> vertex = ltrim_then_split(line);
					ASSERT(vertex.size() >= NUM_COORDINATES);
					Angel::vec3 vertex_angel = {vertex[0], vertex[1], vertex[2]};
					if (vertex.size() > NUM_COORDINATES)
					{
						std::cout << "Warning, loaded ShapeModel has a vertex that had more than " << NUM_COORDINATES << " coordinates!" << std::endl;
					}
					cur_model_coords.push_back(vertex_angel);
					getline(stream, line);
					line_idx_for_cur_shape++;
				} while (line != "\tEND");

				// Process rot
				getline(stream, line);
				line_idx_for_cur_shape++;
				std::vector<float> rotations = ltrim_then_split(line);
				ASSERT(rotations.size() >= NUM_COORDINATES);
				cur_rot = new Angel::vec3{ rotations[0], rotations[1], rotations[2] };
				if (rotations.size() > NUM_COORDINATES)
				{
					std::cout << "Warning, loaded ShapeModel rotation had more than " << NUM_COORDINATES << " coordinates!" << std::endl;
				}

				// Process col
				getline(stream, line);
				line_idx_for_cur_shape++;
				std::vector<float> colors = ltrim_then_split(line);
				ASSERT(colors.size() >= 4);
				cur_col = new Angel::vec4{ colors[0], colors[1], colors[2], colors[3] };
				if (colors.size() > 4)
				{
					std::cout << "Warning, loaded ShapeModel colors had more than 4 values!" << std::endl;
				}

				// Create Shape
				ShapeModel* cur = new ShapeModel(cur_model_coords, cur_col);
				out.push_back(cur);
				line_idx_for_cur_shape = -1;
			}
		}
		else
		{
			line_idx_for_cur_shape = -1;
		}
	}
	return out;
}
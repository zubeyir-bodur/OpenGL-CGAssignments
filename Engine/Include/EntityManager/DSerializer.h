#pragma once
#include "EntityManager/ShapeModel.h"
#include <filesystem>

class DSerializer
{
public:
	static void serialize_drawlist(const std::vector<ShapeModel*>& drawlist, const std::filesystem::path& serialize_path);
	static std::vector<ShapeModel*> deserialize_drawlist(const std::filesystem::path& file_path);
};

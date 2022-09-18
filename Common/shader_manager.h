#pragma once
#include <string>

std::string parse_shader_file(const std::string& file_path);
unsigned int create_program_from_shaders(const std::string& shader);
unsigned int compile_shader(unsigned int type, const std::string& shader_source_code);


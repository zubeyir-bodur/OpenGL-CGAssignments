#pragma once
#include <string>

unsigned int create_program_from_shaders(const std::string vertext_shader, const std::string fragment_shader);
unsigned int compile_shader(unsigned int type, const std::string shader_source_code);

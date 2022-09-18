#include "shader_manager.h"
#include "glew.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string parse_shader_file(const std::string& file_path)
{
	std::ifstream stream(file_path);
	
	std::string line;
	std::stringstream ss;
	while (getline(stream, line))
	{
		ss << line << '\n';
	}
	return ss.str();
}

unsigned int create_program_from_shaders(const std::string& shader)
{
	unsigned int program_id = glCreateProgram();

	//TODO insert the actual OpenGL version here
	auto ss_vertex = std::string("#version 460 core\n#define COMPILING_VS\n");
	auto ss_fragment = std::string("#version 460 core\n#define COMPILING_FS\n");
	ss_vertex += shader;
	ss_fragment += shader;
	unsigned int vertex_shader_id = compile_shader(GL_VERTEX_SHADER, ss_vertex);
	unsigned int fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, ss_fragment);

	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);
	glValidateProgram(program_id);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}

unsigned int compile_shader(unsigned int type, const std::string& shader_source_code)
{
	unsigned int shader_id = glCreateShader(type);

	if (shader_id == GL_INVALID_ENUM)
	{
		return 0;
	}

	const char* src = shader_source_code.c_str();
	glShaderSource(shader_id, 1, &src, nullptr);
	glCompileShader(shader_id);

	int result;

	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
		char* error_msg = static_cast<char*>(alloca(length * sizeof(char)));
		glGetShaderInfoLog(shader_id, length, &length, error_msg);
		std::cout << "Failed to compile "
			<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader!" << std::endl;
		std::cout << error_msg << std::endl;
		glDeleteShader(shader_id);
		return 0;
	}
	return shader_id;
}

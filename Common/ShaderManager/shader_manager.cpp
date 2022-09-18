#include "shader_manager.h"
#include "glew.h"
#include <iostream>

unsigned int create_program_from_shaders(const std::string vertext_shader, const std::string fragment_shader)
{
	unsigned int program_id = glCreateProgram();
	unsigned int vertex_shader_id = compile_shader(GL_VERTEX_SHADER, vertext_shader);
	unsigned int fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);
	glValidateProgram(program_id);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}

unsigned int compile_shader(unsigned int type, const std::string shader_source_code)
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
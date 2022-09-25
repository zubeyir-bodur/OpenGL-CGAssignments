#pragma once
#include "ShaderManager.h"
#include "ErrorManager.h"
#include <glew.h>
#include <GL/GL.h>
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
	unsigned int program_id;
	glCallReturn(glCreateProgram(), program_id);

	std::string ss_vertex, ss_fragment;

	const unsigned int glsl_int_version = get_glsl_version();

	const std::string version_line = "#version " + std::to_string(glsl_int_version) + " core\n";

	ss_vertex = version_line + "#define COMPILING_VS\n" + shader;
	ss_fragment = version_line + "#define COMPILING_FS\n" + shader;

	unsigned int vertex_shader_id = compile_shader(GL_VERTEX_SHADER, ss_vertex);
	unsigned int fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, ss_fragment);

	glCallVoid(glAttachShader(program_id, vertex_shader_id));
	glCallVoid(glAttachShader(program_id, fragment_shader_id));
	glCallVoid(glLinkProgram(program_id));
	glCallVoid(glValidateProgram(program_id));

	glCallVoid(glDeleteShader(vertex_shader_id));
	glCallVoid(glDeleteShader(fragment_shader_id));

	return program_id;
}

unsigned int compile_shader(unsigned int type, const std::string& shader_source_code)
{
	unsigned int shader_id;
	glCallReturn(glCreateShader(type), shader_id);

	if (shader_id == GL_INVALID_ENUM)
	{
		return 0;
	}

	const char* src = shader_source_code.c_str();
	glCallVoid(glShaderSource(shader_id, 1, &src, nullptr));
	glCallVoid(glCompileShader(shader_id));

	int result;

	glCallVoid(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result));

	if (result == GL_FALSE)
	{
		int length;
		glCallVoid(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length));
		char* error_msg = new char[length];
		glCallVoid(glGetShaderInfoLog(shader_id, length, &length, error_msg));
		std::cout << "Failed to compile "
			<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader!" << std::endl;
		std::cout << error_msg << std::endl;
		glCallVoid(glDeleteShader(shader_id));
		delete [] error_msg;
		return 0;
	}
	return shader_id;
}

unsigned int compile_and_bind_shader(const char* path)
{
	std::string program_src = parse_shader_file("../../Common/shaders/triangle.glsl");
	unsigned int program_id = create_program_from_shaders(program_src);
	glCallVoid(glUseProgram(program_id));
	return program_id;
}

unsigned int get_glsl_version()
{
	const unsigned char* glsl_version;
	glCallReturn(glGetString(GL_SHADING_LANGUAGE_VERSION), glsl_version);
	const float glsl_float_version = (const float)std::stof(std::string((const char*)glsl_version));
	return (unsigned int)(glsl_float_version * 100);
}

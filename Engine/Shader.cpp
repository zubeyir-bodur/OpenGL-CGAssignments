#include "Shader.h"
#include "ErrorManager.h"
#include <glew.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

Shader::Shader(const char* path)
{
	m_shader_path = std::filesystem::absolute(path).string();
	const std::string& program_src = parse_shader_file(m_shader_path.c_str());
	m_shader_id = create_program_from_shaders(program_src);
	__glCallVoid(glUseProgram(m_shader_id));
}

Shader::~Shader()
{
	__glCallVoid(glDeleteProgram(m_shader_id));
}

void Shader::bind() const
{
	__glCallVoid(glUseProgram(m_shader_id));
}

void Shader::unbind() const
{
	__glCallVoid(glUseProgram(0));
}

void Shader::set_uniform_1i(const std::string& name, int value)
{
	__glCallVoid(glUniform1i(uniform_location(name), value));
}

void Shader::set_uniform_4f(const std::string& name, float v0, float v1, float v2, float v3)
{
	__glCallVoid(glUniform4f(uniform_location(name), v0, v1, v2, v3));
}

void Shader::set_uniform_mat4f(const std::string& name, Angel::mat4 mat)
{
	__glCallVoid(glUniformMatrix4fv(uniform_location(name), 1, GL_TRUE, &mat[0][0]));
}

int Shader::uniform_location(const std::string& name)
{
	if (m_uniform_location_cache.find(name)
		!= m_uniform_location_cache.end())
	{
		return m_uniform_location_cache[name];
	}
	else
	{
		int loc;
		__glCallReturn(glGetUniformLocation(m_shader_id, name.c_str()), loc);
		if (loc == -1)
		{
			std::cout << "Warning: The uniform " << name
				<< " in " << m_shader_path << " was not found, or it was unused." << std::endl;
		}
		m_uniform_location_cache[name] = loc;
		return loc;
	}
}

std::string Shader::parse_shader_file(const char* file_path)
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

unsigned int Shader::create_program_from_shaders(const std::string& shader)
{
	unsigned int program_id;
	__glCallReturn(glCreateProgram(), program_id);

	std::string ss_vertex, ss_fragment;

	const unsigned int glsl_int_version = get_glsl_version();
	std::string shader_mod;
	const std::string version_line = "#version " + std::to_string(glsl_int_version) + " core\n";
	size_t pos;
	bool version_header_flag;
	if ((pos = shader.find("#version")) != std::string::npos)
	{
		version_header_flag = true;
		shader_mod = shader.substr(shader.substr(pos).find('\n') + 1); // shader code after the version header
	}
	else
	{
		version_header_flag = false;
	}
	ss_vertex = version_line + "#define COMPILING_VS\n" + (version_header_flag ? shader_mod : shader);
	ss_fragment = version_line + "#define COMPILING_FS\n" + (version_header_flag ? shader_mod : shader);

	unsigned int vertex_shader_id = compile_shader(GL_VERTEX_SHADER, ss_vertex);
	unsigned int fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, ss_fragment);

	__glCallVoid(glAttachShader(program_id, vertex_shader_id));
	__glCallVoid(glAttachShader(program_id, fragment_shader_id));
	__glCallVoid(glLinkProgram(program_id));
	__glCallVoid(glValidateProgram(program_id));

	__glCallVoid(glDeleteShader(vertex_shader_id));
	__glCallVoid(glDeleteShader(fragment_shader_id));

	return program_id;
}

unsigned int Shader::compile_shader(unsigned int type, const std::string& shader_source_code)
{
	unsigned int shader_id;
	__glCallReturn(glCreateShader(type), shader_id);

	if (shader_id == GL_INVALID_ENUM)
	{
		return 0;
	}

	const char* src = shader_source_code.c_str();
	__glCallVoid(glShaderSource(shader_id, 1, &src, nullptr));
	__glCallVoid(glCompileShader(shader_id));

	int result;

	__glCallVoid(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result));

	if (result == GL_FALSE)
	{
		int length;
		__glCallVoid(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length));
		char* error_msg = new char[length];
		__glCallVoid(glGetShaderInfoLog(shader_id, length, &length, error_msg));
		std::cout << "Failed to compile "
			<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader!" << std::endl;
		std::cout << error_msg << std::endl;
		__glCallVoid(glDeleteShader(shader_id));
		delete[] error_msg;
		return 0;
	}
	return shader_id;
}

unsigned int Shader::get_glsl_version()
{
	const unsigned char* glsl_version;
	__glCallReturn(glGetString(GL_SHADING_LANGUAGE_VERSION), glsl_version);
	const float glsl_float_version = (const float)std::stof(std::string((const char*)glsl_version));
	return (unsigned int)(glsl_float_version * 100);
}


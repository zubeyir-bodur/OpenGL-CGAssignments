#pragma once
#include <string>
#include <unordered_map>
class Shader
{
private:
	unsigned int m_shader_id;
	std::string m_shader_path;
	std::unordered_map<std::string, int> m_uniform_location_cache;
public:
	Shader(const char* path);
	~Shader();

	void bind() const;
	void unbind() const;

	void set_uniform1i(const std::string& name, int value);
	void set_uniform4f(const std::string& name, float v0, float v1, float v2, float v3);

	static unsigned int get_glsl_version();
private:
	int uniform_location(const std::string& name);

	static std::string parse_shader_file(const char* file_path);
	static unsigned int create_program_from_shaders(const std::string& shader);
	static unsigned int compile_shader(unsigned int type, const std::string& shader_source_code);
};
#pragma once
#include <string>
#include <unordered_map>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
class Shader
{
private:
	unsigned int m_shader_id;
	std::string m_shader_path;
	std::unordered_map<std::string, int> m_uniform_location_cache;
public:
	Shader() {}
	Shader(const char* path);
	~Shader();

	void bind() const;
	void unbind() const;

	void set_uniform_1i(const std::string& name, int value);
	void set_uniform_4f(const std::string& name, float v0, float v1, float v2, float v3);
	void set_uniform_mat4f(const std::string& name, glm::mat4 mat);

	static unsigned int get_glsl_version();
private:
	int uniform_location(const std::string& name);

	static std::string parse_shader_file(const char* file_path);
	static unsigned int create_program_from_shaders(const std::string& shader);
	static unsigned int compile_shader(unsigned int type, const std::string& shader_source_code);
};
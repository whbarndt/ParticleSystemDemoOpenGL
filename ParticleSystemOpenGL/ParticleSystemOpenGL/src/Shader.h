#pragma once
#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
private:
	std::string fpath;
	unsigned int renderer_id;
	std::unordered_map<std::string, unsigned int> uniform_location_cache;
public:
	Shader() = default;
	Shader(const std::string& fpath);
	~Shader();

	void SetShader(const std::string& fpath);
	
	void Bind() const; //But is technically "using program"
	void Unbind() const;

	//set uniforms
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4fv(const std::string& name, const glm::mat4& matrix);
	void SetUniform4fv(const std::string& name, const glm::vec4& vec4);
	
private:
	ShaderProgramSource ParseShader(const std::string& fpath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	unsigned int GetUniformLocation(const std::string& name);
};
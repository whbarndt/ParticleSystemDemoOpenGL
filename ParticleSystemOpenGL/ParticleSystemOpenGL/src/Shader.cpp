#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

Shader::Shader(const std::string& fpath)
	: fpath(fpath), renderer_id(0)
{
    ShaderProgramSource source = ParseShader(fpath);
    renderer_id = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(renderer_id));
}

void Shader::SetShader(const std::string& fpath)
{
    ShaderProgramSource source = ParseShader(fpath);
    renderer_id = CreateShader(source.VertexSource, source.FragmentSource);
}

//Parses the shader from a file based on its "#shader" 
ShaderProgramSource Shader::ParseShader(const std::string& fpath)
{
    std::ifstream stream(fpath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << "\n";
        }
    }

    return { ss[0].str(), ss[1].str() };
};

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    //glShaderSource(*we want the shader*, *how many source codes are we specifying*, *pointer to the pointer*, *length of shader?*)
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    //Error Handling
    //glGetShaderiv(id, *parameter name*, *pass to store result*) i means int and v means vector
    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        //alloc a dynamically allocates memory on the stack
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout <<
            "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << std::endl;
        std::cout << message << std::endl;

        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
};

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    //Using unsigned int instead of GLuint incase I use multiple API's
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    //Deletes the shaders cause they've been linked to a program 
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
};

void Shader::Bind() const
{
    GLCall(glUseProgram(renderer_id));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4fv(const std::string& name, const glm::mat4& matrix)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::SetUniform4fv(const std::string& name, const glm::vec4& vec4)
{
    GLCall(glUniform4fv(GetUniformLocation(name), 1, &vec4[0]));
}

unsigned int Shader::GetUniformLocation(const std::string& name)
{
    if (uniform_location_cache.find(name) != uniform_location_cache.end())
        return uniform_location_cache[name];
    
    GLCall(unsigned int location = glGetUniformLocation(renderer_id, name.c_str()));
    if (location == -1)
        std::cout << "Warning: uniform " << name << "doesn't exist!" << std::endl;
    uniform_location_cache[name] = location;
    return location;

}


#pragma once

#include "GL/glew.h"

#include "Shader.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);
void GLFW_error(int error, const char* description);

class Renderer
{
public:
    void Clear() const;
    void EnableBlend() const;

};
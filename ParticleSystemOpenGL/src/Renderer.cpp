#pragma once

#include <iostream>

#include "Renderer.h"

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "Open GL Error: " << error << " : "
            << function << " : " << file << " : " << line << std::endl;
        return false;
    }
    return true;
}

void GLFW_error(int error, const char* description)
{
    fputs(description, stderr);
}

void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::EnableBlend() const
{
    GLCall(glEnable(GL_BLEND));
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


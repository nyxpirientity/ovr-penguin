#include "gl_context.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace nyxpiri::ovrpenguin
{
GlContext::GlContext()
{
    glfwWindowHint(GLFW_VISIBLE, 0);
    window = glfwCreateWindow(1, 1, "gl_context", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
}

GlContext::~GlContext()
{
    glfwDestroyWindow(window);
}

void GlContext::bind()
{
    glfwMakeContextCurrent(window);
}

void GlContext::unbind()
{
    glfwMakeContextCurrent(nullptr);
}

}// namespace nyxpiri::ovrpenguin
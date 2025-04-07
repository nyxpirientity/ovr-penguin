#ifndef GL_CONTEXT_HPP
#define GL_CONTEXT_HPP
#include "scene/node.hpp"

struct GLFWwindow;

namespace nyxpiri::ovrpenguin
{
class GlContext : public Node
{
public:
    GlContext();
    ~GlContext();

    void bind();
    void unbind();

private:
    GLFWwindow* window = nullptr;
};
}// namespace nyxpiri::ovrpenguin
#endif // GL_CONTEXT_HPP
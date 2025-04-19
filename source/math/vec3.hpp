#ifndef VEC3_HPP
#define VEC3_HPP
#include "math/num_types.hpp"
#include "glm/vec3.hpp"

namespace nyxpiri::ovrpenguin
{
using GlmVec3 = glm::vec<3, real, glm::defaultp>;
struct Vec3 : public GlmVec3
{
private:
    using Super = GlmVec3;
public:
    Vec3() : Super(0.0, 0.0, 0.0) {}
    Vec3(real scalar) : Super(scalar) {}
    Vec3(real x, real y, real z) : Super(x, y, z) {}
};
} // namespace nyxpiri::ovrpenguin
#endif // VEC3_HPP

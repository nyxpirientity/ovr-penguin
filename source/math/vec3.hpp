#ifndef VEC3_HPP
#define VEC3_HPP
#include "math/num_types.hpp"
#include "glm/vec3.hpp"

namespace nyxpiri::ovrpenguin
{
struct Vec3 : public glm::vec<3, real, glm::defaultp>
{
private:
    using Super = glm::vec<3, real, glm::defaultp>;
public:
    Vec3() : Super(0.0, 0.0, 0.0) {}
    Vec3(real scalar) : Super(scalar) {}
    Vec3(real x, real y, real z) : Super(x, y, z) {}
};
} // namespace nyxpiri::ovrpenguin
#endif // VEC3_HPP

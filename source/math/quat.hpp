#ifndef QUAT_HPP
#define QUAT_HPP
#include "num_types.hpp"
#include "glm/gtc/quaternion.hpp"
#include <cmath>

namespace nyxpiri::ovrpenguin
{
struct Quat : public glm::qua<real>
{
private:
    using Super = glm::qua<real>;
public:
    Quat()
    : Super(1.0, 0.0, 0.0, 0.0) {}

    Quat(real w, real x, real y, real z)
    : Super(w, x, y, z) {}

    Quat(const Vec3& euler_angles)
    : Super(euler_angles) {}

    static const Quat IDENTITY;
};

const Quat Quat::IDENTITY = Quat{};

} // namespace nyxpiri::ovrpenguin
#endif // QUAT_HPP

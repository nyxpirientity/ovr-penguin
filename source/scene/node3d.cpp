#include "node3d.hpp"

namespace nyxpiri::ovrpenguin
{
Node3d::Node3d() {}
const Mat4x4& Node3d::get_transform()
{
    return transform;
}

void Node3d::set_transform(const Mat4x4& in_transform)
{
    transform = in_transform;
}
} // namespace nyxpiri::ovrpenguin

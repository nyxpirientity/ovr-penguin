#ifndef NODE3D_HPP
#define NODE3D_HPP

#include "math/mat4x4.hpp"
#include "node.hpp"

namespace nyxpiri::ovrpenguin
{
class Node3d : public Node
{
public:
    Node3d();

    const Mat4x4& get_transform();
    void set_transform(const Mat4x4& in_transform);

private:
    Mat4x4 transform;
};
} // namespace nyxpiri::ovrpenguin

#endif // NODE3D_HPP

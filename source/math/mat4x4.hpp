#ifndef MAT4X4_HPP
#define MAT4X4_HPP

#include "math/vec4.hpp"
#include "types/nyxarrays.hpp"

namespace nyxpiri::ovrpenguin
{
struct Mat4x4
{
    Array<Vec4, 4> rows;
};
} // namespace nyxpiri::ovrpenguin
#endif // MAT4X4_HPP

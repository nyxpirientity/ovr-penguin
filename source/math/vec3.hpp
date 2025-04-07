#ifndef VEC3_HPP
#define VEC3_HPP
#include "math/num_types.hpp"
namespace nyxpiri::ovrpenguin
{
struct Vec3
{
    Vec3(): z(0.0), y(0.0), x(0.0) {}

    real z;
    real y;
    real x;
};
} // namespace nyxpiri::ovrpenguin
#endif // VEC3_HPP

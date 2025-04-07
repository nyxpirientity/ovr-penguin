#ifndef VEC2_HPP
#define VEC2_HPP
#include "math/num_types.hpp"
namespace nyxpiri::ovrpenguin
{
struct Vec2
{
    Vec2(): z(0.0), y(0.0) {}

    real z;
    real y;
};
} // namespace nyxpiri::ovrpenguin
#endif // VEC2_HPP

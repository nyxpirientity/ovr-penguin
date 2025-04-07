#ifndef VEC4_HPP
#define VEC4_HPP
#include "math/num_types.hpp"
namespace nyxpiri::ovrpenguin
{
struct Vec4
{
    Vec4(): z(0.0), y(0.0), x(0.0), w(0.0) {}

    real z;
    real y;
    real x;
    real w;
};
} // namespace nyxpiri::ovrpenguin
#endif // VEC4_HPP

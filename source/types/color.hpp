#ifndef COLOR_HPP
#define COLOR_HPP
#include "math/num_types.hpp"

namespace nyxpiri::ovrpenguin
{
class Color
{
public:
    Color()
    : r(0), g(0), b(0), a(0)
    {
    }
    Color(u8 in_r, u8 in_g, u8 in_b, u8 in_a)
    : r(in_r), g(in_g), b(in_b), a(in_a)
    {
    }

    Color(f32 in_r, f32 in_g, f32 in_b, f32 in_a)
    : r(in_r * 255), g(in_g * 255), b(in_b * 255), a(in_a * 255)
    {
    }

    Color(f64 in_r, f64 in_g, f64 in_b, f64 in_a)
    : r(in_r * 255), g(in_g * 255), b(in_b * 255), a(in_a * 255)
    {
    }

union
{
    u8 rgba[4];
    struct
    {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
};

private:
};

// Color is expected to be exactly 4 bytes, so that an array of Color objects will be a valid image, for example.
static_assert(sizeof(Color) == 4);

} // namespace nyxpiri::ovrpenguin
#endif // COLOR_HPP
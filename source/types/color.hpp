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
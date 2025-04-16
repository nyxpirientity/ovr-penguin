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
    {
        r = static_cast<f32>(in_r) / 255.0f;
        g = static_cast<f32>(in_g) / 255.0f;
        b = static_cast<f32>(in_b) / 255.0f;
        a = static_cast<f32>(in_a) / 255.0f;
    }

    Color(f32 in_r, f32 in_g, f32 in_b, f32 in_a)
    : r(in_r), g(in_g), b(in_b), a(in_a)
    {
        
    }

union
{
    f32 rgba[4];
    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
};

private:
};

// Color is expected to be exactly 16 bytes, so that an array of Color objects will be a valid image, for example.
static_assert(sizeof(Color) == 16);

class Color8
{
public:
    Color8()
    : r(0), g(0), b(0), a(0)
    {
    }
    Color8(u8 in_r, u8 in_g, u8 in_b, u8 in_a)
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
};

static_assert(sizeof(Color8) == 4);

} // namespace nyxpiri::ovrpenguin
#endif // COLOR_HPP
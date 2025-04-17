#ifndef MAT4X4_HPP
#define MAT4X4_HPP

#include "types/nyxarrays.hpp"
#include "math/num_types.hpp"

namespace nyxpiri::ovrpenguin
{
struct Mat4x4
{
    Mat4x4()
    {
        for (usize i = 0; i < 4; i++)
        {
            for (usize j = 0; j < 4; j++)
            {
                data[i][j] = 0.0;
            }
        }
    }

    Mat4x4(real a, real b, real c, real d,
           real e, real f, real g, real h,
           real i, real j, real k, real l,
           real m, real n, real o, real p)
    {
        data[0] = {a, b, c, d};
        data[1] = {e, f, g, h};
        data[2] = {i, j, k, l};
        data[3] = {m, n, o, p};
    }

    Array<Array<real, 4>, 4> data;

    Array<real, 4>& operator[](usize i)
    {
        return data[i];
    }
};
} // namespace nyxpiri::ovrpenguin
#endif // MAT4X4_HPP

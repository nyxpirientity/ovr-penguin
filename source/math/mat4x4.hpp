#ifndef MAT4X4_HPP
#define MAT4X4_HPP

#include "math/num_types.hpp"
#include "glm/mat4x4.hpp"

namespace nyxpiri::ovrpenguin
{
struct Mat4x4 : public glm::mat<4, 4, real, glm::defaultp>
{
private:
    using Super = glm::mat<4, 4, real, glm::defaultp>;
public:
    Mat4x4()
    : Super(
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0}
    ) {}

    Mat4x4(col_type a, col_type b, col_type c, col_type d)
    : Super(a, b, c ,d) {}
};
} // namespace nyxpiri::ovrpenguin
#endif // MAT4X4_HPP

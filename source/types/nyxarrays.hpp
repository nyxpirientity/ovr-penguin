#ifndef NYXARRAYS_HPP
#define NYXARRAYS_HPP

#include <vector>
#include <array>

namespace nyxpiri::ovrpenguin
{
template<typename T>
using DynArray = std::vector<T>;

template<typename T, std::size_t length>
using Array = std::array<T, length>;

} // namespace nyxpiri::ovrpenguin

#endif // NYXARRAYS_HPP

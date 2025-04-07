#ifndef NYXMEMORY_HPP
#define NYXMEMORY_HPP
#include <memory>

namespace nyxpiri::ovrpenguin
{
template<typename T>
using OwnerPtr = std::unique_ptr<T>;

template<typename T>
using WeakPtr = T*;

template<typename T>
using UnownedPtr = T*;

} // namespace nyxpiri::ovrpenguin

#endif // NYXMEMORY_HPP

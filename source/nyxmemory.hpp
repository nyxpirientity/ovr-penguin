#ifndef NYXMEMORY_HPP
#define NYXMEMORY_HPP
#include <memory>

namespace nyxpiri::ovrpenguin
{
template<typename T>
using OwnerPtr = std::unique_ptr<T>;

template<typename T>
using UnownedPtr = T*;

template<typename T>
class WeakPtr
{
public:
    WeakPtr(): ptr(nullptr) {};
    WeakPtr(const OwnerPtr<T>& in_ptr): ptr(in_ptr.get()) {}
    explicit WeakPtr(T* in_ptr): ptr(in_ptr) {}
    WeakPtr(std::nullptr_t): ptr(nullptr) {}

    ~WeakPtr() = default;
    WeakPtr(const WeakPtr<T>& other) = default;
    WeakPtr(WeakPtr<T>&& other) = default;
    WeakPtr<T>& operator=(const WeakPtr<T>& other) = default;
    WeakPtr<T>& operator=(WeakPtr<T>&& other) = default;
    WeakPtr<T>& operator=(std::nullptr_t) { ptr = nullptr; return *this; }

    [[nodiscard]] T* get_raw() const
    {
        return ptr;
    }

    [[nodiscard]] T* operator->() const
    {
        return ptr;
    }

    [[nodiscard]] T& operator*() const
    {
        return *ptr;
    }

    [[nodiscard]] bool is_valid() const
    {
        return ptr != nullptr;
    }

    [[nodiscard]] operator bool() const
    {
        return is_valid();
    }

    [[nodiscard]] bool operator==(const WeakPtr<T>& other) const
    {
        return ptr == other.ptr;
    }

    [[nodiscard]] bool operator!=(const WeakPtr<T>& other) const
    {
        return ptr != other.ptr;
    }

    [[nodiscard]] bool operator==(const T* other) const
    {
        return ptr == other;
    }

    [[nodiscard]] bool operator!=(const T* other) const
    {
        return ptr != other;
    }

    [[nodiscard]] bool operator==(const OwnerPtr<T>& other) const
    {
        return ptr == other.get();
    }

    [[nodiscard]] bool operator!=(const OwnerPtr<T>& other) const
    {
        return ptr != other.get();
    }

private:
    T* ptr = nullptr;
};

} // namespace nyxpiri::ovrpenguin

#endif // NYXMEMORY_HPP

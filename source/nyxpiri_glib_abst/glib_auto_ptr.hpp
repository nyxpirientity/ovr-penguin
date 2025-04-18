#ifndef GLIB_UNIQUE_PTR_H
#define GLIB_UNIQUE_PTR_H

#include <glib.h>

namespace nyxpiri::ovrpenguin
{
template <typename T>
class GLibAutoPtr {
public:
    GLibAutoPtr(T* in_ptr) : ptr(in_ptr)
    {
        if (!ptr)
        {
            return;
        }

        g_object_ref(ptr);
    }

    ~GLibAutoPtr()
    {
        if (!ptr)
        {
            return;
        }

        g_object_unref(ptr);
    }

    GLibAutoPtr(const GLibAutoPtr&) = delete;
    GLibAutoPtr& operator=(const GLibAutoPtr&) = delete;

    GLibAutoPtr(GLibAutoPtr&& other) noexcept : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }

    GLibAutoPtr& operator=(GLibAutoPtr&& other) noexcept
    {
        if (this != &other)
        {
            if (ptr)
            {
                g_object_unref(ptr);
            }

            ptr = other.ptr;
            other.ptr = nullptr;
        }

        return *this;
    }

    operator bool() const
    {
        return get();
    }

    T& operator*()
    {
        return *(get());
    }

    const T& operator*() const
    {
        return *(get());
    }

    T* operator->()
    {
        return get();
    }

    const T* operator->() const
    {
        return get();
    }

    T* get()
    {
        return ptr;
    }

    T* get() const
    {
        return ptr;
    }

    T* release()
    {
        T const* old_ptr = ptr;

        ptr = nullptr;

        return old_ptr;
    }

    void reset(T const* ptr = nullptr)
    {
        if (ptr)
        {
            g_object_unref(ptr);
        }

        ptr = ptr;
    }

private:
    T* ptr;
};
} // nyxpiri::ovrpenguin
#endif // GLIB_UNIQUE_PTR_H

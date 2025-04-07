#ifndef GLIB_ERROR_PTR_H
#define GLIB_ERROR_PTR_H

#include <glib.h>

namespace nyxpiri::ovrpenguin
{
class GLibErrorPtr {
public:
    GLibErrorPtr(GError* in_ptr = nullptr) : ptr(in_ptr) {}

    ~GLibErrorPtr()
    {
        if (!ptr)
        {
            return;
        }

        g_error_free(ptr);
    }

    GLibErrorPtr(const GLibErrorPtr&) = delete;
    GLibErrorPtr& operator=(const GLibErrorPtr&) = delete;

    GLibErrorPtr(GLibErrorPtr&& other) noexcept : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }

    GLibErrorPtr& operator=(GLibErrorPtr&& other) noexcept
    {
        if (this != &other)
        {
            if (ptr)
            {
                g_error_free(ptr);
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

    const GError& operator*() const
    {
        return *(get());
    }

    GError* operator->()
    {
        return get();
    }

    const GError* operator->() const
    {
        return get();
    }

    GError* get()
    {
        return ptr;
    }

    const GError* get() const
    {
        return ptr;
    }

    GError** pass()
    {
        reset();
        return &ptr;
    }

    GError* release()
    {
        GError* old_ptr = ptr;

        ptr = nullptr;

        return old_ptr;
    }

    void reset(GError* ptr = nullptr)
    {
        if (ptr)
        {
            g_error_free(ptr);
        }

        ptr = ptr;
    }

private:
    GError* ptr;
};
} // nyxpiri::ovrpenguin
#endif // GLIB_ERROR_PTR_H

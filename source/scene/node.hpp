#ifndef NODE_H
#define NODE_H

#include "nyxmemory.hpp"
#include <queue>
#include "math/num_types.hpp"
#include "types/nyxarrays.hpp"

namespace nyxpiri::ovrpenguin
{
class Node
{
public:
    Node();
    virtual ~Node() = default;

    template<typename T, typename... Args>
    static inline UnownedPtr<T> construct(Args... args)
    {
        return new T(args...);
    }

    template<typename T>
    static inline void destroy(UnownedPtr<T> ptr)
    {
        delete ptr;
    }

    const DynArray<OwnerPtr<Node>>& get_children();

    template<typename T>
    WeakPtr<T> adopt(UnownedPtr<T> new_child)
    {
        adoption_queue.push(OwnerPtr<Node>{new_child});
        new_child->parent = WeakPtr<Node>{this};

        return WeakPtr<T>{new_child};
    }

    void queue_destroy();

    WeakPtr<Node> get_parent();

    void start();
    void tick(real delta_seconds);
    void stop();

    bool is_started() const;

    void stop_tree();
    
    usize find_child_index(WeakPtr<Node> child);

    virtual void set_max_fps(real val);
    virtual real get_max_fps();

protected:
    virtual void on_start() {}
    virtual void on_tick(real delta_seconds) {}
    virtual void on_stop() {}

private:
    [[nodiscard]] UnownedPtr<Node> unadopt(usize index);
    [[nodiscard]] UnownedPtr<Node> unadopt(WeakPtr<Node> child);

private:
    DynArray<OwnerPtr<Node>> children;
    std::queue<OwnerPtr<Node>> adoption_queue;
    std::queue<WeakPtr<Node>> unadoption_queue;
    bool destroy_queued = false;
    WeakPtr<Node> parent = nullptr;

    bool started = false;
};

} // namespace nyxpiri::ovrpenguin
#endif // NODE_H

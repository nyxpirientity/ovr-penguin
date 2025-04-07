#ifndef NODE_H
#define NODE_H

#include "nyxmemory.hpp"
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

    const DynArray<OwnerPtr<Node>>& get_children();

    template<typename T>
    WeakPtr<T> adopt(UnownedPtr<T> new_child)
    {
        new_child->parent = this;
        children.push_back(OwnerPtr<Node>(new_child));
        return WeakPtr<T>{new_child};
    }

    [[nodiscard]] UnownedPtr<Node> unadopt(usize index);

    WeakPtr<Node> get_parent();

    void start();
    void tick(real delta_seconds);
    void stop();

    bool is_started() const;

    void stop_tree();

protected:
    virtual void on_start() {}
    virtual void on_tick(real delta_seconds) {}
    virtual void on_stop() {}

private:
    DynArray<OwnerPtr<Node>> children;
    WeakPtr<Node> parent = nullptr;

    bool started = false;
};

} // namespace nyxpiri::ovrpenguin
#endif // NODE_H

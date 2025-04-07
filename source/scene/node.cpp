#include "node.hpp"

namespace nyxpiri::ovrpenguin
{

Node::Node() {}

const DynArray<OwnerPtr<Node>>& Node::get_children()
{
    return children;
}

UnownedPtr<Node> Node::unadopt(usize index)
{
    UnownedPtr<Node> child = children[index].release();

    child->parent = nullptr;
    children.erase(children.begin() + index);

    return child;
}

WeakPtr<Node> Node::get_parent()
{
    return parent;
}

void Node::start()
{
    for(const OwnerPtr<Node>& child : get_children())
    {
        child->start();
    }

    started = true;

    on_start();
}

void Node::tick(real delta_seconds)
{
    for(const OwnerPtr<Node>& child : get_children())
    {
        child->tick(delta_seconds);
    }

    on_tick(delta_seconds);
}

void Node::stop()
{
    for(const OwnerPtr<Node>& child : get_children())
    {
        child->stop();
    }

    started = false;

    on_stop();
}

bool Node::is_started() const
{
    return started;
}

void Node::stop_tree()
{
    if (parent)
    {
        parent->stop_tree();
        return;
    }

    stop();
}

} // namespace nyxpiri::ovrpenguin


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

    if (child->is_started())
    {
        child->stop();
    }

    return child;
}

UnownedPtr<Node> Node::unadopt(WeakPtr<Node> child)
{
    usize index = find_child_index(child);
    
    if (index == -1)
    {
        return nullptr;
    }

    return unadopt(index);
}

void Node::queue_destroy()
{
    if (!parent)
    {
        return;
    }

    destroy_queued = true;
    parent->unadoption_queue.push(WeakPtr<Node>{this});
}

WeakPtr<Node> Node::get_parent()
{
    return parent;
}

void Node::start()
{
    for(const OwnerPtr<Node>& child : get_children())
    {
        if (child->is_started())
        {
            continue;
        }

        child->start();
    }

    started = true;

    on_start();
}

void Node::tick(real delta_seconds)
{
    while(not unadoption_queue.empty())
    {
        usize child_index = find_child_index(unadoption_queue.front());
        unadoption_queue.pop();

        UnownedPtr<Node> child = unadopt(child_index);

        if(child->destroy_queued)
        {
            Node::destroy(child);
        }
    }

    while(not adoption_queue.empty())
    {
        UnownedPtr<Node> new_child = adoption_queue.front().release();
        adoption_queue.pop(); 
        children.push_back(OwnerPtr<Node>(new_child));

        if (is_started())
        {
            new_child->start();
        }
    }

    for(const OwnerPtr<Node>& child : get_children())
    {
        if (not child->is_started())
        {
            continue;
        }

        child->tick(delta_seconds);
    }

    on_tick(delta_seconds);
}

void Node::stop()
{
    for(const OwnerPtr<Node>& child : get_children())
    {
        if (not child->is_started())
        {
            continue;
        }

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

usize Node::find_child_index(WeakPtr<Node> child)
{
    for (usize i = 0; i < children.size(); i++)
    {
        if (child == children[i])
        {
            return i;
        }
    }

    return -1;
}

} // namespace nyxpiri::ovrpenguin


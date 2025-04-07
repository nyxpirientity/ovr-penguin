#ifndef EVENT_HPP
#define EVENT_HPP

#include "math/num_types.hpp"
#include "types/nyxarrays.hpp"
#include <cassert>
#include <cstring>
#include <functional>
#include <stack>

namespace nyxpiri::ovrpenguin
{
class EventBinding
{
public:
    EventBinding() = default;

    EventBinding(const EventBinding& other) = delete;
    EventBinding& operator=(const EventBinding& other) = delete;
    EventBinding(EventBinding&& other)
    {
        other.event_data->set_binding_ptr(other.event_data->event, other.index, this);
        event_data = other.event_data;
        index = other.index;
        other.invalidate();
    }

    EventBinding& operator=(EventBinding&& other)
    {
        if (is_bound())
        {
            unbind();
        }

        other.event_data->set_binding_ptr(other.event_data->event, other.index, this);
        event_data = other.event_data;
        index = other.index;
        other.invalidate();

        return *this;
    }

    ~EventBinding()
    {
        unbind();
    }

    bool is_bound() const
    {
        return event_data and index != -1;
    }

    void unbind()
    {
        if (not is_bound())
        {
            return;
        }

        event_data->unbind_function(event_data->event, *this);
    }

private:
    template<typename ...Args>
    friend class Event;

    struct EventData
    {
        void* event = nullptr;
        void(*unbind_function)(void* event, EventBinding& binding);
        void(*set_binding_ptr)(void* event, usize index, EventBinding* new_ptr);
    };

    void invalidate()
    {
        index = -1;
        event_data = nullptr;
    }

private:
    EventData* event_data = nullptr;
    usize index = -1;
};

template<typename ...Args>
class Event
{
public:
    Event() = default;

    Event(Event&& other)
    {
        for(EventListener& listener : other.listeners)
        {
            if (listener.binding == nullptr)
            {
                continue;
            }

            listener.binding->event_data = &binding_event_data;
        }

        free_list = std::move(other.free_list);
        listeners = std::move(other.listeners);
    }

    Event& operator=(Event&& other)
    {
        unbind_all();

        for(EventListener& listener : other.listeners)
        {
            if (listener.binding == nullptr)
            {
                continue;
            }

            listener.binding->event_data = &binding_event_data;
        }

        free_list = std::move(other.free_list);
        listeners = std::move(other.listeners);

        return *this;
    }

    Event(const Event& other) = delete;
    Event& operator=(const Event& other) = delete;

    ~Event()
    {
        unbind_all();
    }

    void unbind_all()
    {
        for(EventListener& listener : listeners)
        {
            if (listener.binding == nullptr or listener.callback == nullptr)
            {
                continue;
            }

            listener.binding->unbind();
        }
    }

    void bind(EventBinding& binding, const std::function<void(Args...)>& callback)
    {
        if (binding.is_bound())
        {
            binding.unbind();
        }

        binding.event_data = &binding_event_data;

        if (free_list.size() > 0)
        {
            binding.index = free_list.top();
            free_list.pop();
        }
        else
        {
            binding.index = listeners.size();
            listeners.push_back({});
        }

        listeners[binding.index].callback = callback;
        listeners[binding.index].binding = &binding;
    }

    void unbind(EventBinding& binding)
    {
        if (binding.event_data != &binding_event_data or binding.index == -1)
        {
            return;
        }

        free_list.push(binding.index);
        listeners[binding.index].binding = nullptr;
        listeners[binding.index].callback = nullptr;
        binding.invalidate();
    }

    void broadcast(Args... args)
    {
        for (const EventListener& listener : listeners)
        {
            if (listener.binding == nullptr)
            {
                continue;
            }

            listener.callback(args...);
        }
    }

private:
    void set_binding_ptr(usize index, EventBinding* new_ptr)
    {
        listeners[index].binding = new_ptr;
    }

    static inline void static_set_binding_ptr(void* event, usize index, EventBinding* new_ptr)
    {
        reinterpret_cast<Event<Args...>*>(event)->set_binding_ptr(index, new_ptr);
    }

    static inline void static_unbind(void* event, EventBinding& binding)
    {
        reinterpret_cast<Event<Args...>*>(event)->unbind(binding);
    }

    struct EventListener
    {
        std::function<void(Args...)> callback;
        EventBinding* binding = nullptr;
    };

    DynArray<EventListener> listeners;
    std::stack<usize> free_list;
    EventBinding::EventData binding_event_data{.event = this, .unbind_function = static_unbind, .set_binding_ptr = static_set_binding_ptr};
};
} // namespace nyxpiri::ovrpenguin

#endif // EVENT_HPP

#ifndef STDIN_LISTENER_HPP
#define STDIN_LISTENER_HPP

#include "scene/node.hpp"
#include "types/mutex.hpp"
#include "types/event.hpp"
#include <atomic>
#include <thread>
#include <string>

namespace nyxpiri::ovrpenguin
{

class StdIoHandler : public Node
{
    using Super = Node;
public:
    /* Notifies the io thread to wait for input. Returns an event that will be called on the main thread with the input sent by the user. Must be called from the main thread.*/
    Event<std::string>& async_await_input();

    /* Prints a string to std out asynchronously (on the io thread). Must be called on the main thread.*/
    void async_print_string(const std::string& string);

public:
    StdIoHandler();
    ~StdIoHandler() = default;

    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;

private:
    std::thread io_thread;
    std::atomic_bool io_thread_enabled = true;

    DynArray<Event<std::string>> input_events;

    SharedMutex<std::string> input_buffer;
    SharedMutex<std::string> output_buffer;

    std::atomic_size_t num_input_awaits = 0;
};

} // namespace nyxpiri::ovrpenguin

#endif // STDIN_LISTENER_HPP

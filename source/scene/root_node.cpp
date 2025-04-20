#include "root_node.hpp"
#include <thread>

namespace nyxpiri::ovrpenguin
{

RootNode::RootNode() {}

void RootNode::on_start()
{
    Super::on_start();

    previous_tick_time_point = std::chrono::steady_clock::now();

    while(is_started())
    {
        real delta_seconds = std::chrono::duration<real>(std::chrono::steady_clock::now() - previous_tick_time_point).count();
        previous_tick_time_point = std::chrono::steady_clock::now();

        tick(delta_seconds);
        std::this_thread::sleep_for(min_delta_microseconds - (std::chrono::steady_clock::now() - previous_tick_time_point));
    }
}

void RootNode::set_max_fps(real max_fps)
{
    if (max_fps <= 0)
    {
        min_delta_microseconds = std::chrono::microseconds{0};
        return;
    }

    min_delta_microseconds = std::chrono::microseconds{usize((1.0 / max_fps) * 1'000'000)};
}

real RootNode::get_max_fps()
{
    return 1.0 / (real(min_delta_microseconds.count()) / 1'000'000);
}

} // namespace nyxpiri::ovrpenguin

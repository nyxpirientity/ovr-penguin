#include "root_node.hpp"
#include <thread>

namespace nyxpiri::ovrpenguin {

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
        std::this_thread::sleep_for(std::chrono::milliseconds(8) - (std::chrono::steady_clock::now() - previous_tick_time_point));
    }
}

} // namespace nyxpiri::ovrpenguin

#ifndef ROOT_NODE_HPP
#define ROOT_NODE_HPP

#include "scene/node.hpp"
#include <chrono>

namespace nyxpiri::ovrpenguin {

class RootNode : public Node
{
private: using Super = Node;

public:
    RootNode();

protected:
    virtual void on_start() override;

    virtual void set_max_fps(real max_fps) override;
    virtual real get_max_fps() override;

protected:
    std::chrono::steady_clock::time_point previous_tick_time_point;
    std::chrono::microseconds min_delta_microseconds = std::chrono::microseconds{12500};
};

} // namespace nyxpiri::ovrpenguin

#endif // ROOT_NODE_HPP

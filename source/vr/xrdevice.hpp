#ifndef XRDEVICE_HPP
#define XRDEVICE_HPP

#include "scene/node3d.hpp"

namespace nyxpiri::ovrpenguin
{
class XRDevice : public Node3d
{
    using Super = Node3d;
public:
    XRDevice();

    virtual void on_tick(real delta_seconds) override;
};

} // namespace nyxpiri::ovrpenguin

#endif // XRDEVICE_HPP

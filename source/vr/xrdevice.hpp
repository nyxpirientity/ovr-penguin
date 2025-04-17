#ifndef XRDEVICE_HPP
#define XRDEVICE_HPP

#include "scene/node3d.hpp"
#include <openvr.h>

namespace nyxpiri::ovrpenguin
{
enum class XrDeviceType : u8
{
    HeadMountedDisplay,
    LeftMotionController,
    RightMotionController
};

class XrDevice : public Node3d
{
    using Super = Node3d;
public:
    XrDevice(WeakPtr<class OvrRuntime> in_ovr_runtime);
    
    virtual void on_tick(real delta_seconds) override;
    XrDeviceType get_device_type();

private:
    XrDeviceType type = XrDeviceType::HeadMountedDisplay;
    WeakPtr<class OvrRuntime> ovr_runtime = nullptr;
};

} // namespace nyxpiri::ovrpenguin

#endif // XRDEVICE_HPP

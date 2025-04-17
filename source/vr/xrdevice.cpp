#include "xrdevice.hpp"
#include <openvr.h>

namespace nyxpiri::ovrpenguin
{
XrDevice::XrDevice(WeakPtr<class OvrRuntime> in_ovr_runtime) 
: ovr_runtime(in_ovr_runtime)
{
}

void XrDevice::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);
}
XrDeviceType XrDevice::get_device_type()
{
    return type;
}
} // namespace nyxpiri::ovrpenguin

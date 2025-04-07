#include "ovr_runtime.hpp"
#include <openvr.h>

namespace nyxpiri::ovrpenguin
{

OvrRuntime::OvrRuntime(WeakPtr<StdIoHandler> in_io_handler)
{
    io_handler = in_io_handler;
}

void OvrRuntime::on_start()
{
    Super::on_start();
}

void OvrRuntime::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);
}

void OvrRuntime::on_stop()
{
    Super::on_stop();

    if (is_running())
    {
        vr::VR_Shutdown();
        vr_system = nullptr;
    }
}

void OvrRuntime::initialize()
{
    if (is_running())
    {
        return;
    }
    
    vr::EVRInitError error;
    vr_system = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Overlay);

    if (error != vr::EVRInitError::VRInitError_None)
    {
        vr_system = nullptr;
        return;
    }
}

bool OvrRuntime::is_running()
{
    return vr_system;
}

} // namespace nyxpiri::ovrpenguin

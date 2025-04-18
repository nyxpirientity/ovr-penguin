#include "ovr_runtime.hpp"
#include <openvr.h>

namespace nyxpiri::ovrpenguin
{
struct OvrRuntime::OvrData
{
    struct TrackedDeviceData
    {
        vr::TrackedDevicePose_t pose;
        usize ovr_device_index = 0;
        Mat4x4 matrix;
    };

    Array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> raw_tracked_device_poses;
    DynArray<TrackedDeviceData> tracked_devices;
};

OvrRuntime::OvrRuntime(WeakPtr<StdIoHandler> in_io_handler)
{
    ovr_data = new OvrData;
    io_handler = in_io_handler;
}

OvrRuntime::~OvrRuntime()
{
    delete ovr_data;
    ovr_data = nullptr;
}

void OvrRuntime::on_start()
{
    Super::on_start();
}

void OvrRuntime::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);

    if (!is_running())
    {
        return;
    }
    
    ovr_data->tracked_devices.clear();
    ovr_data->tracked_devices.reserve(vr::k_unMaxTrackedDeviceCount);
    vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0.0, &(ovr_data->raw_tracked_device_poses[0]), ovr_data->raw_tracked_device_poses.size());
    vr::k_unTrackedDeviceIndex_Hmd;
    vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
    vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
    for (usize i = 0; i < ovr_data->raw_tracked_device_poses.size(); i++)
    {
        vr::TrackedDevicePose_t& pose = ovr_data->raw_tracked_device_poses[i];

        if (!pose.bPoseIsValid)
        {
            continue;
        }
        
        OvrData::TrackedDeviceData device;
        vr::HmdMatrix34_t ovr_matrix = device.pose.mDeviceToAbsoluteTracking; 
        device.ovr_device_index = i;
        device.matrix = Mat4x4{
            ovr_matrix.m[0][0], ovr_matrix.m[1][0], ovr_matrix.m[2][0], 0.0,
            ovr_matrix.m[0][1], ovr_matrix.m[1][1], ovr_matrix.m[2][1], 0.0,
            ovr_matrix.m[0][2], ovr_matrix.m[1][2], ovr_matrix.m[2][2], 0.0,
            ovr_matrix.m[0][3], ovr_matrix.m[1][3], ovr_matrix.m[2][3], 1.0
        };
        
        ovr_data->tracked_devices.push_back(device);
    }
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

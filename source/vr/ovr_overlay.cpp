#include "ovr_overlay.hpp"
#include "openvr.h"
#include "graphics/gl_context.hpp"
#include <glad/gl.h>
#include <cassert>
#include "math/quat.hpp"
#include "math/mat4x4.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

namespace nyxpiri::ovrpenguin
{
OvrOverlay::OvrOverlay(WeakPtr<GlContext> in_gl_context)
{
    gl_context = in_gl_context;
    vr_texture = new vr::Texture_t; 
}

OvrOverlay::~OvrOverlay()
{
    delete vr_texture;
}

void OvrOverlay::on_start()
{
    Super::on_start();

    gl_context->bind();
    glCreateTextures(GL_TEXTURE_2D, 1, &gl_texture_id);
    
    vr_texture->eColorSpace = vr::EColorSpace::ColorSpace_Gamma;
    vr_texture->eType = vr::ETextureType::TextureType_OpenGL;
    vr_texture->handle = (void*)(uintptr_t)(gl_texture_id);
    gl_context->unbind();
    
    DynArray<Color> default_texture_data = {{(u8)164, (u8)64, (u8)255, (u8)255}, {(u8)164, (u8)64, (u8)255, (u8)128}, {(u8)164, (u8)64, (u8)255, (u8)64}, {(u8)164, (u8)64, (u8)255, (u8)32}};

    set_texture_data(std::move(default_texture_data), 2, 2);
}

void OvrOverlay::on_stop()
{
    Super::on_stop();
    vr_texture->handle = nullptr;

    glDeleteTextures(1, &gl_texture_id);
    destroy_ovr_overlay();
}

void OvrOverlay::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);

    vr::TrackedDeviceIndex_t device_index = -1;
    vr::HmdMatrix34_t ovr_transform;
    Mat4x4 transform;
    Quat quaternion;
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::angle(quaternion), glm::axis(quaternion));

    ovr_transform.m[0][0] = transform[0][0];
    ovr_transform.m[1][0] = transform[1][0];
    ovr_transform.m[2][0] = transform[2][0];
    
    ovr_transform.m[0][1] = transform[0][1];
    ovr_transform.m[1][1] = transform[1][1];
    ovr_transform.m[2][1] = transform[2][1];

    ovr_transform.m[0][2] = transform[0][2];
    ovr_transform.m[1][2] = transform[1][2];
    ovr_transform.m[2][2] = transform[2][2];

    ovr_transform.m[0][3] = transform[0][3];
    ovr_transform.m[1][3] = transform[1][3];
    ovr_transform.m[2][3] = transform[2][3];

    switch (parent)
    {
    case OverlayParent::HeadMountedDisplay:
        device_index = vr::k_unTrackedDeviceIndex_Hmd;
        [[fallthrough]];
    
    case OverlayParent::LeftMotionController:
        device_index = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
        [[fallthrough]];
    
    case OverlayParent::RightMotionController:
        device_index = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
        vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(handle, device_index, &ovr_transform);
        break;
    
    case OverlayParent::PlaySpace:
        vr::VROverlay()->SetOverlayTransformAbsolute(handle, vr::ETrackingUniverseOrigin::TrackingUniverseStanding, &ovr_transform);
        break;
    }
}

OvrOverlay::Type OvrOverlay::get_overlay_type() const
{
    return overlay_type;
}

bool OvrOverlay::is_null() const
{
    return overlay_type == Type::null or vr::VROverlay() == nullptr;
}

void OvrOverlay::destroy_ovr_overlay()
{
    if (is_null())
    {
        return;
    }
    
    vr::VROverlay()->DestroyOverlay(handle);
    overlay_type = Type::null;
    handle = -1;
}

bool OvrOverlay::set_overlay_type(Type new_type)
{
    if (get_overlay_type() != Type::null)
    {
        destroy_ovr_overlay();
    }

    vr::EVROverlayError creation_error;
    
    switch (new_type)
    {
    case Type::null:
        return true;

    case Type::dashboard:
        creation_error = vr::VROverlay()->CreateDashboardOverlay(overlay_name.c_str(), overlay_name.c_str(), &handle, &thumbnail_handle);
        break;
        
    case Type::world:
        creation_error = vr::VROverlay()->CreateOverlay(overlay_name.c_str(), overlay_name.c_str(), &handle);
        break;
    }

    if (creation_error != vr::EVROverlayError::VROverlayError_None)
    {
        // TODO: Figure out if we should destroy it when there's an error? I assume if there was an error creating it, it won't exist, but I'm unsure.
        return false;
    }

    overlay_type = new_type;

    return true;
}

void OvrOverlay::set_texture_data(DynArray<Color>&& data, usize width, usize height)
{
    texture_data = std::move(data);
    data.clear();
    data.reserve(data.size());

    for (usize i = 0; i < height / 2; i++)
    {
        for (usize j = 0; j < width; j++)
        {
            usize top_index = i * width + j;
            usize bottom_index = (height - i - 1) * width + j;

            Color temp = texture_data[top_index];
            texture_data[top_index] = texture_data[bottom_index];
            texture_data[bottom_index] = temp;
        }
    }
    
    gl_context->bind();

    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture_data[0]);
    
    if (get_overlay_type() == Type::null)
    {
        return;
    }

    if (not vr::VROverlay())
    {
        return;
    }

    vr::VROverlayError texture_error = vr::VROverlay()->SetOverlayTexture(handle, vr_texture);

    gl_context->unbind();
}

void OvrOverlay::set_size(f64 new_size)
{
    size = new_size;
    refresh_overlay_properties();
}

void OvrOverlay::set_curve(f64 new_curve)
{
    curve = new_curve;
    refresh_overlay_properties();
}

void OvrOverlay::refresh_overlay_properties()
{
    vr::VROverlay()->SetOverlayWidthInMeters(handle, size);
    vr::VROverlay()->SetOverlayCurvature(handle, curve);
}

bool OvrOverlay::set_overlay_name(const std::string &in_name)
{
    overlay_name = in_name;

    if (is_null())
    {
        return true;
    }

    vr::VROverlayError name_error = vr::VROverlay()->SetOverlayName(handle, in_name.c_str());
    
    if (name_error != vr::VROverlayError::VROverlayError_None)
    {
        return false;
    }

    return true;
}

void OvrOverlay::set_overlay_parent(OverlayParent new_parent)
{
    parent = new_parent;
}

void OvrOverlay::set_overlay_position(const Vec3 &new_pos)
{
    position = new_pos;
}

void OvrOverlay::set_overlay_rotation(const Vec3 &new_rot)
{
    rotation = new_rot;
}

} // namespace nyxpiri::ovrpenguin

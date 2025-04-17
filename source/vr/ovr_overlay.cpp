#include "ovr_overlay.hpp"
#include "openvr.h"
#include "graphics/gl_context.hpp"
#include <glad/gl.h>
#include <cassert>
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
    
    Color default_texture_data[4] = {{(u8)164, (u8)64, (u8)255, (u8)255}, {(u8)164, (u8)64, (u8)255, (u8)128}, {(u8)164, (u8)64, (u8)255, (u8)64}, {(u8)164, (u8)64, (u8)255, (u8)32}};

    set_texture_data(&default_texture_data[0], 2, 2);
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

    //Color default_texture_data[4] = {{(u8)164, (u8)64, (u8)255, (u8)255}, {(u8)164, (u8)64, (u8)255, (u8)128}, {(u8)164, (u8)64, (u8)255, (u8)64}, {(u8)164, (u8)64, (u8)255, (u8)32}};

    //set_texture_data(&default_texture_data[0], 2, 2);
}

OvrOverlay::Type OvrOverlay::get_overlay_type() const
{
    return overlay_type;
}

bool OvrOverlay::is_null() const
{
    return overlay_type == Type::null;
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

void OvrOverlay::set_texture_data(const Color* const data, usize width, usize height)
{
    texture_data.resize(width * height);
    
    std::memcpy(&texture_data[0], data, width * height * sizeof(Color));

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

} // namespace nyxpiri::ovrpenguin

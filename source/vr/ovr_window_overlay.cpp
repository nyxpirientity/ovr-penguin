#include "ovr_window_overlay.hpp"
#include "openvr.h"
#include "graphics/gl_context.hpp"
#include "media/screen_capturer.hpp"
#include <glad/gl.h>
#include <cassert>
#include <cstring>

namespace nyxpiri::ovrpenguin
{
OvrWindowOverlay::OvrWindowOverlay(WeakPtr<class GlContext> in_gl_context, WeakPtr<class ScreenCapturer> in_screen_capturer)
: Super(in_gl_context), screen_capturer(in_screen_capturer)
{
}

OvrWindowOverlay::~OvrWindowOverlay()
{
}

void OvrWindowOverlay::reset_window_session()
{
    end_window_session();
    
    screen_capture_stream = screen_capturer->create_stream();
    screen_capture_stream->on_data_received.bind(screen_capture_data_received_binding, [this](DynArray<Color>& color, usize width, usize height)
    {
        for (const ColorKey& key : color_keys)
        {
            for (Color& color : color)
            {
                //const i16 r_diff = std::abs(i16(color.r) - i16(key.color.r));    
                //const i16 g_diff = std::abs(i16(color.g) - i16(key.color.g));    
                //const i16 b_diff = std::abs(i16(color.b) - i16(key.color.b));
                //const i16 squared_dist = (r_diff + g_diff + b_diff);

                //const f64 normalized_dist = f64(squared_dist) / f64(765);

                //color.a = u8(std::max(0.0, std::min(1.0, math::normalize_to_range(normalized_dist, key.min_dist, key.max_dist))) * 255);
                // ^^ that implementation is potentially faster, but also potentially less accurate. keeping it to test later.

                const GlmVec3 color_vec = {real(color.r) / 255, real(color.g) / 255, real(color.b) / 255};
                const GlmVec3 key_color_vec = {real(key.color.r) / 255, real(key.color.g) / 255, real(key.color.b) / 255};
                const real normalized_dist = glm::distance(color_vec, key_color_vec) / glm::length(GlmVec3{1.0, 1.0, 1.0});

                color.a = u8(std::max(0.0, std::min(1.0, math::normalize_to_range(normalized_dist, key.min_dist, key.max_dist))) * 255);
            }
        }
        
        if (left_crop != 0.0 or right_crop != 0.0 or top_crop != 0.0 or bottom_crop != 0.0)
        {
            const usize new_width = width - std::min(width, (usize(left_crop) + usize(right_crop)));
            const usize new_height = height - std::min(height, (usize(top_crop) + usize(bottom_crop)));

            if (new_width == 0 or new_height == 0)
            {
                return;
            }

            DynArray<Color> cropped_color(new_width * new_height);

            for (usize y = 0; y < new_height; y++)
            {
                for (usize x = 0; x < new_width; x++)
                {
                    cropped_color[y * new_width + x] = color[(y + top_crop) * width + (x + left_crop)];
                }
            }

            color = std::move(cropped_color);
            width = new_width;
            height = new_height;
        }

        set_texture_data(std::move(color), width, height);
        color.clear();
    });

    screen_capture_stream->on_screencast_started.bind(screen_capture_session_started_binding, [this](Result<> result)
    {
        on_window_session_started.broadcast(result);
    });
}

void OvrWindowOverlay::end_window_session()
{
    if (!screen_capture_stream)
    {
        return;
    }

    screen_capturer->destroy_stream(screen_capture_stream);
    screen_capture_stream = nullptr;
}

usize OvrWindowOverlay::get_num_color_keys()
{
    return color_keys.size();
}

usize OvrWindowOverlay::new_color_key()
{
    color_keys.push_back({});
    return color_keys.size() - 1;
}

void OvrWindowOverlay::set_color_key_color(usize index, Color color)
{
    color_keys[index].color = color;
}

void OvrWindowOverlay::set_color_key_min(usize index, real min)
{
    color_keys[index].min_dist = min;
}

void OvrWindowOverlay::set_color_key_max(usize index, real max)
{
    color_keys[index].max_dist = max;
}

void OvrWindowOverlay::destroy_color_key(usize index)
{
    color_keys.erase(color_keys.begin() + index);
}

std::string OvrWindowOverlay::get_color_key_string(usize index) const
{
    const ColorKey& key = color_keys[index];
    return 
        "--color " + std::to_string(real(key.color.r) / 255) + " " + std::to_string(real(key.color.g) / 255) + " " + std::to_string(real(key.color.b) / 255) + 
        " --min " + std::to_string(key.min_dist) + 
        " --max " + std::to_string(key.max_dist);
}

usize OvrWindowOverlay::get_top_crop()
{
    return top_crop;
}

void OvrWindowOverlay::set_top_crop(usize val)
{
    top_crop = val;
}

usize OvrWindowOverlay::get_bottom_crop()
{
    return bottom_crop;
}

void OvrWindowOverlay::set_bottom_crop(usize val)
{
    bottom_crop = val;
}

usize OvrWindowOverlay::get_right_crop()
{
    return right_crop;
}

void OvrWindowOverlay::set_right_crop(usize val)
{
    right_crop = val;
}

usize OvrWindowOverlay::get_left_crop()
{
    return left_crop;
}

void OvrWindowOverlay::set_left_crop(usize val)
{
    left_crop = val;
}

void OvrWindowOverlay::on_start()
{
    Super::on_start();
}

void OvrWindowOverlay::on_stop()
{
    Super::on_stop();

    end_window_session();
}

void OvrWindowOverlay::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);
}

} // namespace nyxpiri::ovrpenguin

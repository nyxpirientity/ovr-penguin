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
                const i16 r_diff = std::abs(i16(color.r) - i16(key.color.r));    
                const i16 g_diff = std::abs(i16(color.g) - i16(key.color.g));    
                const i16 b_diff = std::abs(i16(color.b) - i16(key.color.b));
                const i16 squared_dist = (r_diff + g_diff + b_diff);

                const f64 normalized_dist = f64(squared_dist) / f64(765);

                color.a = u8(std::max(0.0, std::min(1.0, math::normalize_to_range(normalized_dist, key.min_dist, key.max_dist))) * 255);
            }
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

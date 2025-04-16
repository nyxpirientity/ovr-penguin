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
    screen_capture_stream = screen_capturer->create_stream();
    screen_capture_stream->on_data_received.bind(screen_capture_data_received_binding, [this](const DynArray<Color> color, usize width, usize height)
    {
        set_texture_data(color.data(), width, height);
    });
}

void OvrWindowOverlay::on_start()
{
    Super::on_start();
}

void OvrWindowOverlay::on_stop()
{
    Super::on_stop();
}

void OvrWindowOverlay::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);
}

} // namespace nyxpiri::ovrpenguin

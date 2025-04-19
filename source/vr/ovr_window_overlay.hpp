#ifndef OVR_WINDOW_OVERLAY_HPP
#define OVR_WINDOW_OVERLAY_HPP

#include "scene/node3d.hpp"
#include "types/color.hpp"
#include "types/event.hpp"
#include "ovr_overlay.hpp"
#include "types/result.hpp"

namespace vr
{
    class Texture_t;
}
namespace nyxpiri::ovrpenguin
{
class OvrWindowOverlay : public OvrOverlay
{
    using Super = OvrOverlay;
public:
    OvrWindowOverlay(WeakPtr<class GlContext> in_gl_context, WeakPtr<class ScreenCapturer> in_screen_capturer);
    ~OvrWindowOverlay();

    Event<Result<>> on_window_session_started;

    void reset_window_session();
    void end_window_session();

protected:
    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;
    
private:
    struct ColorKey
    {
        Color color;
        f64 min_dist = 0.01;
        f64 max_dist = 0.02;
    };

private:
    WeakPtr<class ScreenCapturer> screen_capturer = nullptr;
    WeakPtr<class ScreenCaptureStream> screen_capture_stream = nullptr;

    EventBinding screen_capture_data_received_binding;
    EventBinding screen_capture_session_started_binding;

    DynArray<ColorKey> color_keys;
};

} // namespace nyxpiri::ovrpenguin

#endif // OVR_WINDOW_OVERLAY_HPP

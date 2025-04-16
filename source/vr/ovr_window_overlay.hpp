#ifndef OVR_WINDOW_OVERLAY_HPP
#define OVR_WINDOW_OVERLAY_HPP

#include "scene/node3d.hpp"
#include "types/color.hpp"
#include "types/event.hpp"
#include "ovr_overlay.hpp"

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

    void reset_window_session();

protected:
    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;
    
private:

private:
    WeakPtr<class ScreenCapturer> screen_capturer = nullptr;
    WeakPtr<class ScreenCaptureStream> screen_capture_stream = nullptr;

    EventBinding screen_capture_data_received_binding;
};

} // namespace nyxpiri::ovrpenguin

#endif // OVR_WINDOW_OVERLAY_HPP

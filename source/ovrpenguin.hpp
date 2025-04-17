#ifndef OVRPENGUIN_HPP
#define OVRPENGUIN_HPP
#include "scene/node.hpp"
#include "types/event.hpp"
#include "diagnostics/logger.hpp"

namespace nyxpiri::ovrpenguin
{
class OvrPenguin : public Node
{
    using Super = Node;
public:
    OvrPenguin();
    ~OvrPenguin();

    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;

    void execute_command(const std::string& input);

    WeakPtr<class OvrWindowOverlay> get_overlay_by_name(const std::string& name);
    usize get_overlay_index_by_name(const std::string& name);

    void init_overlay_capture(WeakPtr<class OvrWindowOverlay> overlay);

    void set_overlay_type(WeakPtr<class OvrWindowOverlay> overlay, const std::string& type_str);

private:
    void init_next_overlay_capture();

private:
    Logger logger;
    WeakPtr<class StdIoHandler> io_handler = nullptr;
    WeakPtr<class OvrRuntime> ovr_runtime = nullptr;
    WeakPtr<class GlContext> gl_context = nullptr;
    WeakPtr<class ScreenCapturer> screen_capturer = nullptr;
    DynArray<WeakPtr<class OvrWindowOverlay>> overlays;
    usize current_window_overlay_capture_init_index = -1;
    bool wait_for_input = true;

    EventBinding input_event_binding;
    EventBinding init_window_overlay_capture_log_binding;
    EventBinding init_all_window_overlay_capture_binding;
};

} // namespace nyxpiri::ovrpenguin

#endif
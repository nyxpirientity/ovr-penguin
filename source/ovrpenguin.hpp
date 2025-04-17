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

private:
    Logger logger;
    WeakPtr<class StdIoHandler> io_handler = nullptr;
    WeakPtr<class OvrRuntime> ovr_runtime = nullptr;
    WeakPtr<class GlContext> gl_context = nullptr;
    WeakPtr<class ScreenCapturer> screen_capturer = nullptr;
    DynArray<WeakPtr<class OvrWindowOverlay>> overlays;

    EventBinding input_event_binding;
};

} // namespace nyxpiri::ovrpenguin

#endif
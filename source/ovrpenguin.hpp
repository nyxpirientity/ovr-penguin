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

private:
    Logger logger;
    WeakPtr<class StdIoHandler> io_handler = nullptr;
    WeakPtr<class OvrRuntime> ovr_runtime = nullptr;
    WeakPtr<class GlContext> gl_context = nullptr;
    WeakPtr<class ScreenCapturer> screen_capturer = nullptr;
    WeakPtr<class OvrWindowOverlay> test_overlay = nullptr;

    EventBinding input_event_binding;
};

} // namespace nyxpiri::ovrpenguin

#endif
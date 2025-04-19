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

    void refresh_aliases();

    std::string serialize_state_to_exec();

    void destroy_overlay(usize overlay_index);
    
    void reset_state();

private:
    void init_next_overlay_capture();

    void run_if_cmd_option_vec3(const class StringCommand& command, const std::string& option, usize from_index, std::function<void(struct Vec3 val)> function);
    void run_if_cmd_option_real(const class StringCommand& command, const std::string& option, usize from_index, std::function<void(real val)> function);
    void run_if_cmd_option_usize(const class StringCommand& command, const std::string& option, usize from_index, std::function<void(usize val)> function);

private:
    Logger logger;
    WeakPtr<class StdIoHandler> io_handler = nullptr;
    WeakPtr<class OvrRuntime> ovr_runtime = nullptr;
    WeakPtr<class GlContext> gl_context = nullptr;
    WeakPtr<class ScreenCapturer> screen_capturer = nullptr;
    DynArray<WeakPtr<class OvrWindowOverlay>> overlays;
    usize current_window_overlay_capture_init_index = -1;
    bool wait_for_input = true;
    /* List of aliases, first pair element is the alias name (what the user types to invoke the alias), the second is the actual alias value */
    DynArray<std::pair<std::string, std::string>> aliases;

    EventBinding input_event_binding;
    EventBinding init_window_overlay_capture_log_binding;
    EventBinding init_all_window_overlay_capture_binding;
};

} // namespace nyxpiri::ovrpenguin

#endif
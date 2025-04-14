#ifndef SCREEN_CAPTURER_HPP
#define SCREEN_CAPTURER_HPP

#include "scene/node.hpp"
#include "types/event.hpp"
#include "types/color.hpp"
#include "math/num_types.hpp"

#include "nyxpiri_glib_abst/glib_auto_ptr.hpp"
#include "nyxpiri_glib_abst/glib_error_ptr.hpp"
#include "nyxpiri_glib_abst/glib_print_utils.hpp"

#include <pipewire/pipewire.h>
#include <libportal/portal.h>

namespace nyxpiri::ovrpenguin
{
class ScreenCaptureStream
{
public:
    ScreenCaptureStream(WeakPtr<class ScreenCapturer> capturer, class Logger& in_logger);
    ~ScreenCaptureStream();

public:
    Event<void(DynArray<Color> data, usize width, usize height)> on_data_recieved;

public:
    void debug_log_info();

protected:
    void on_screencast_session_created(GObject* source_object, GAsyncResult* res);
    void on_screencast_session_start(GObject* source_object, GAsyncResult* res);

    void on_stream_state_changed(pw_stream_state old, pw_stream_state state, const char* error);
    void on_stream_param_changed(u32 id, const spa_pod* param);
    void on_process();

    static void static_on_screencast_session_created(GObject* source_object, GAsyncResult* res, void* data);
    static void static_on_screencast_session_start(GObject* source_object, GAsyncResult* res, void* data);
    
    static void static_on_stream_state_changed(void *data, pw_stream_state old, pw_stream_state state, const char *error);
    static void static_on_stream_param_changed(void* data, u32 id, const spa_pod* param);
    static void static_on_stream_process(void* data);

private:
    WeakPtr<class ScreenCapturer> capturer = nullptr;
    pw_core* m_pw_core = nullptr;
    pw_stream* m_pw_stream = nullptr;
    pw_stream_events m_pw_stream_events;
    spa_hook m_pw_stream_events_listener;
    class Logger& logger;
    XdpSession* session = nullptr;
};

class ScreenCapturer : public Node
{
using Super = Node;
public:
    ScreenCapturer(class Logger& in_logger);
    ~ScreenCapturer();

    const GLibAutoPtr<XdpPortal>& get_xdp_portal();
    pw_context* get_pw_context() { return m_pw_context;}
    
    WeakPtr<ScreenCaptureStream> create_stream();

protected:
    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;

private:
    pw_loop* m_pw_loop = nullptr;
    pw_context* m_pw_context = nullptr;
    GLibAutoPtr<XdpPortal> xdp_portal = nullptr;
    class Logger& logger;
    GMainContext* g_main_context = nullptr;
    DynArray<OwnerPtr<ScreenCaptureStream>> streams;
};
}
#endif
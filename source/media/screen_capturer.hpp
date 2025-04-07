#ifndef SCREEN_CAPTURER_HPP
#define SCREEN_CAPTURER_HPP

#include "scene/node.hpp"
#include "types/event.hpp"
#include "types/color.hpp"
#include "math/num_types.hpp"

#include "nyxpiri_glib_abst/glib_auto_ptr.hpp"
#include "nyxpiri_glib_abst/glib_error_ptr.hpp"
#include "nyxpiri_glib_abst/glib_print_utils.hpp"

#include <libportal/portal.h>


struct pw_loop;
struct pw_context;
struct pw_core;

namespace nyxpiri::ovrpenguin
{
class ScreenCaptureStream
{
public:
    ScreenCaptureStream(WeakPtr<class ScreenCapturer> capturer, WeakPtr<class Logger> in_logger);
    ~ScreenCaptureStream();

public:
    Event<void(DynArray<Color> data, usize width, usize height)> on_data_recieved;

protected:
    void on_screencast_session_created(GObject* source_object, GAsyncResult* res);
    void on_screencast_session_start(GObject* source_object, GAsyncResult* res);

    static void static_on_screencast_session_created(GObject* source_object, GAsyncResult* res, void* data);
    static void static_on_screencast_session_start(GObject* source_object, GAsyncResult* res, void* data);
    
private:
    WeakPtr<class ScreenCapturer> capturer = nullptr;
    pw_core* m_pw_core = nullptr;
    WeakPtr<Logger> logger = nullptr;
};

class ScreenCapturer : public Node
{
using Super = Node;
public:
    ScreenCapturer(WeakPtr<class Logger> in_logger);
    ~ScreenCapturer();

    const GLibAutoPtr<XdpPortal>& get_xdp_portal();
    
protected:
    virtual void on_start() override;
    virtual void on_tick(real delta_seconds) override;
    virtual void on_stop() override;

private:
    pw_loop* m_pw_loop = nullptr;
    pw_context* m_pw_context = nullptr;
    GLibAutoPtr<XdpPortal> xdp_portal = nullptr;
    WeakPtr<class Logger> logger = nullptr;
    GMainContext* g_main_context = nullptr;
};
}
#endif
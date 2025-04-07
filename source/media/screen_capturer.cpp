#include "screen_capturer.hpp"
#include "diagnostics/logger.hpp"
#include <pipewire/pipewire.h>
#include <libportal/portal.h>

namespace nyxpiri::ovrpenguin
{
ScreenCapturer::ScreenCapturer(WeakPtr<Logger> in_logger)
{
    logger = in_logger;
}

ScreenCapturer::~ScreenCapturer()
{
}

const GLibAutoPtr<XdpPortal>& ScreenCapturer::get_xdp_portal()
{
    return xdp_portal;
}

void ScreenCapturer::on_start()
{
    Super::on_start();

    m_pw_loop = pw_loop_new(NULL);
    pw_loop_enter(m_pw_loop);

    m_pw_context = pw_context_new(m_pw_loop, NULL, 0);

    pw_properties* core_props = pw_properties_new(
        PW_KEY_MEDIA_ROLE, "Screen",
        PW_KEY_MEDIA_TYPE, "Video",
        PW_KEY_MEDIA_CATEGORY, "Capture"
        );

    g_main_context = g_main_context_default();
    GLibErrorPtr obj_creation_error = nullptr;
    xdp_portal = xdp_portal_initable_new(obj_creation_error.pass());

    if (!xdp_portal)
    {
        logger->log_error("ScreenCapturer", format_gerror_string("Failed to create XdpPortal object :c attempting to stop execution...", *obj_creation_error), true);
        stop_tree();
        return;
    }

    //pw_core* core = pw_context_connect_fd(m_pw_context, xdp_session_open_pipewire_remote(), core_props, 0);
    
}

void ScreenCapturer::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);

    pw_loop_iterate(m_pw_loop, 0);
    g_main_context_iteration(g_main_context, false);
}

void ScreenCapturer::on_stop()
{
    pw_loop_leave(m_pw_loop);
    pw_loop_destroy(m_pw_loop);
    
    pw_context_destroy(m_pw_context);
}

ScreenCaptureStream::ScreenCaptureStream(WeakPtr<ScreenCapturer> in_capturer, WeakPtr<class Logger> in_logger)
: capturer(in_capturer), logger(in_logger)
{
    xdp_portal_create_screencast_session(capturer->get_xdp_portal().get(),
    XdpOutputType(XDP_OUTPUT_MONITOR | XDP_OUTPUT_VIRTUAL | XDP_OUTPUT_WINDOW),
    XdpScreencastFlags(XDP_SCREENCAST_FLAG_NONE),
    XdpCursorMode(XDP_CURSOR_MODE_EMBEDDED),
    XdpPersistMode(XDP_PERSIST_MODE_TRANSIENT),
    NULL, NULL, static_on_screencast_session_created, this);
}

ScreenCaptureStream::~ScreenCaptureStream()
{
    pw_core_disconnect(m_pw_core);
}

void ScreenCaptureStream::on_screencast_session_created(GObject *source_object, GAsyncResult *res)
{
    GLibErrorPtr error;
    GLibAutoPtr<XdpSession> session = xdp_portal_create_screencast_session_finish(XDP_PORTAL(source_object), res, error.pass());

    if (error)
    {
        // TODO: Probably a better way to handle this?
        logger->log_error("ScreenCaptureStream", format_gerror_string("Screencast XdpSession creation failed :c", *error), true);
        return;
    }

    logger->log("ScreenCaptureStream", "Starting XdpSession!", true);

    xdp_session_start(session.get(), NULL, NULL, static_on_screencast_session_start, this);
}

void ScreenCaptureStream::on_screencast_session_start(GObject *source_object, GAsyncResult *res)
{
    GLibErrorPtr error;
    XdpSession* session = XDP_SESSION(source_object);
    xdp_session_start_finish(session, res, error.pass());

    if (error)
    {
        std::cerr << format_gerror_string("Screencast XdpSession start failed :c", *error);
        return;
    }

    XdpSessionType session_type = XdpSessionType(xdp_session_get_session_type(session));

    if (session_type != XDP_SESSION_SCREENCAST)
    {
        std::cerr << "Somehow got wrong XdpSessionType? :c" << session_type << "\n";
        return;
    }

    std::cout << "XdpSession started!\n";
    std::cout << "...Closing XdpSession!\n";
    xdp_session_close(session);
}

void ScreenCaptureStream::static_on_screencast_session_start(GObject *source_object, GAsyncResult *res, void *data)
{
    reinterpret_cast<ScreenCaptureStream*>(data)->on_screencast_session_start(source_object, res);
}

void ScreenCaptureStream::static_on_screencast_session_created(GObject *source_object, GAsyncResult *res, gpointer data)
{
    reinterpret_cast<ScreenCaptureStream*>(data)->on_screencast_session_created(source_object, res);
}
}
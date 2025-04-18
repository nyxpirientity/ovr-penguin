#include "screen_capturer.hpp"
#include "diagnostics/logger.hpp"
#include <pipewire/pipewire.h>
#include <libportal/portal.h>
#include <spa/param/video/type-info.h>
#include <spa/param/video/format-utils.h>
#include <spa/param/props.h>

namespace nyxpiri::ovrpenguin
{
ScreenCapturer::ScreenCapturer(Logger& in_logger)
: logger(in_logger)
{
}

ScreenCapturer::~ScreenCapturer()
{
}

const GLibAutoPtr<XdpPortal>& ScreenCapturer::get_xdp_portal()
{
    return xdp_portal;
}

WeakPtr<ScreenCaptureStream> ScreenCapturer::create_stream()
{
    streams.push_back(OwnerPtr<ScreenCaptureStream>{new ScreenCaptureStream(WeakPtr<ScreenCapturer>{this}, logger)});
    return streams[streams.size() - 1];
}

void ScreenCapturer::destroy_stream(WeakPtr<ScreenCaptureStream>& stream)
{
    for (usize i = 0; i < streams.size(); i++)
    {
        if (stream != streams[i])
        {
            continue;
        }

        streams.erase(streams.begin() + i);
        break;
    }

    stream = nullptr;
}

void ScreenCapturer::on_start()
{
    Super::on_start();

    m_pw_loop = pw_loop_new(NULL);
    pw_loop_enter(m_pw_loop);

    m_pw_context = pw_context_new(m_pw_loop, NULL, 0);

    //pw_properties* core_props = pw_properties_new(
        //PW_KEY_MEDIA_ROLE, "Screen",
        //PW_KEY_MEDIA_TYPE, "Video",
        //PW_KEY_MEDIA_CATEGORY, "Capture", NULL
        //);

    g_main_context = g_main_context_default();
    GLibErrorPtr obj_creation_error = nullptr;
    xdp_portal = xdp_portal_initable_new(obj_creation_error.pass());

    if (!xdp_portal)
    {
        logger.log_error("ScreenCapturer", format_gerror_string("Failed to create XdpPortal object :c attempting to stop execution...", *obj_creation_error), true);
        stop_tree();
        return;
    }
}

void ScreenCapturer::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);

    pw_loop_iterate(m_pw_loop, 0);
    g_main_context_iteration(g_main_context, false);
}

void ScreenCapturer::on_stop()
{
    Super::on_stop();

    streams.clear();

    pw_loop_leave(m_pw_loop);

    pw_context_destroy(m_pw_context);
    pw_loop_destroy(m_pw_loop);
}

ScreenCaptureStream::ScreenCaptureStream(WeakPtr<ScreenCapturer> in_capturer, Logger& in_logger)
: capturer(in_capturer), logger(in_logger)
{
    xdp_portal_create_screencast_session(capturer->get_xdp_portal().get(),
    XdpOutputType(XDP_OUTPUT_MONITOR | XDP_OUTPUT_VIRTUAL | XDP_OUTPUT_WINDOW),
    XdpScreencastFlags(XDP_SCREENCAST_FLAG_NONE),
    XdpCursorMode(XDP_CURSOR_MODE_EMBEDDED),
    XdpPersistMode(XDP_PERSIST_MODE_TRANSIENT),
    NULL, NULL, static_on_screencast_session_created, this);
    
    m_pw_stream_events = {
        .version = PW_VERSION_STREAM_EVENTS,
        .state_changed = static_on_stream_state_changed,
        .param_changed = static_on_stream_param_changed,
        .process = static_on_stream_process,
    };
}

ScreenCaptureStream::~ScreenCaptureStream()
{
    logger.log("ScreenCaptureStream", "...closing XdpSession, pw_stream and pw_core!", false);
    xdp_session_close(session.get());
    pw_stream_disconnect(m_pw_stream);
    pw_core_disconnect(m_pw_core);
}

void ScreenCaptureStream::debug_log_info()
{
    logger.log("ScreenCaptureStream", "Debug log info!", true);
    
    if (session)
    {
        logger.log("ScreenCaptureStream", "XdpSession state is " + std::to_string(xdp_session_get_session_state(session.get())), true);
    }
}

void ScreenCaptureStream::on_screencast_session_created(GObject *source_object, GAsyncResult *res)
{
    GLibErrorPtr error;
    session = xdp_portal_create_screencast_session_finish(XDP_PORTAL(source_object), res, error.pass());

    if (error)
    {
        // TODO: Probably a better way to handle this?
        logger.log_error("ScreenCaptureStream", format_gerror_string("Screencast XdpSession creation failed :c", *error), true);
        on_screencast_started.broadcast({format_gerror_string("Screencast XdpSession creation failed :c", *error), -1});
        return;
    }

    logger.log("ScreenCaptureStream", "starting XdpSession!", false);

    xdp_session_start(session.get(), NULL, NULL, static_on_screencast_session_start, this);
}

void ScreenCaptureStream::on_screencast_session_start(GObject *source_object, GAsyncResult *res)
{
    GLibErrorPtr error;
    xdp_session_start_finish(session.get(), res, error.pass());

    if (error)
    {
        logger.log_error("ScreenCaptureStream", format_gerror_string("Screencast XdpSession start failed :c", *error), true);
        on_screencast_started.broadcast({format_gerror_string("Screencast XdpSession start failed :c", *error), -1});
        return;
    }

    XdpSessionType session_type = XdpSessionType(xdp_session_get_session_type(session.get()));

    if (session_type != XDP_SESSION_SCREENCAST)
    {
        logger.log_error("ScreenCaptureStream", std::string("Somehow got wrong XdpSessionType? :c '") + std::to_string(session_type), true);
        on_screencast_started.broadcast({format_gerror_string("Somehow got wrong XdpSessionType? :c '" + std::to_string(session_type), *error), -1});
        return;
    }

    logger.log("ScreenCaptureStream", "XdpSession started!", false);
    
    XdpSessionState session_state = xdp_session_get_session_state(session.get());
    if (session_state == XdpSessionState::XDP_SESSION_ACTIVE)
    {
        logger.log("ScreenCaptureStream", "Current XdpSession status is active!", false);
    }
    else
    {
        logger.log_error("ScreenCaptureStream", "Current XdpSession status isn't active... :c", true);
        on_screencast_started.broadcast({format_gerror_string("Current XdpSession status isn't active... :c", *error), -1});
        return;
    }

    GVariant* xdp_session_streams = xdp_session_get_streams(session.get());
    
    if (g_variant_n_children(xdp_session_streams) <= 0)
    {
        logger.log_error("ScreenCaptureStream", "No streams in XdpSession? :c", true);
        on_screencast_started.broadcast({format_gerror_string("No streams in XdpSession? :c", *error), -1});
        return;
    }

    GVariant* xdp_pw_node_id = g_variant_get_child_value(g_variant_get_child_value(xdp_session_streams, 0), 0);

    if (not g_variant_is_of_type(xdp_pw_node_id, G_VARIANT_TYPE_UINT32))
    {
        logger.log_error("ScreenCaptureStream", "XdpSession attempt to get pipewire node ID returned a non-uint32 value? :c", true);
        on_screencast_started.broadcast({format_gerror_string("XdpSession attempt to get pipewire node ID returned a non-uint32 value? :c", *error), -1});
        return;
    }

    u32 pw_node_id = g_variant_get_uint32(xdp_pw_node_id);
    logger.log("ScreenCaptureStream", "Pipewire node ID is " + std::to_string(pw_node_id), false);

    pw_properties* stream_props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Video",
        PW_KEY_MEDIA_CATEGORY, "Capture",
        PW_KEY_MEDIA_ROLE, "Screen",
        NULL);
    
    m_pw_core = pw_context_connect_fd(capturer->get_pw_context(), xdp_session_open_pipewire_remote(session.get()), nullptr, 0); 

    spa_zero(m_spa_video_info);

    m_pw_stream = pw_stream_new(m_pw_core, "ovr-penguin", stream_props);
    const spa_pod *params[1];
    u8 buffer[1024];
    spa_pod_builder spa_pod_builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    params[0] = static_cast<const spa_pod*>(spa_pod_builder_add_object(&spa_pod_builder,
        SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
        SPA_FORMAT_mediaType,       SPA_POD_Id(SPA_MEDIA_TYPE_video),
        SPA_FORMAT_mediaSubtype,    SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
        SPA_FORMAT_VIDEO_format,    SPA_POD_CHOICE_ENUM_Id(3,
                                        SPA_VIDEO_FORMAT_RGB,
                                        SPA_VIDEO_FORMAT_RGBA,
                                        SPA_VIDEO_FORMAT_BGRx
                                        )));
        
    spa_zero(m_pw_stream_events_listener);
    pw_stream_add_listener(m_pw_stream, &m_pw_stream_events_listener, &m_pw_stream_events, this);
    
    int connect_result = pw_stream_connect(m_pw_stream, PW_DIRECTION_INPUT, pw_node_id, pw_stream_flags(PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_AUTOCONNECT), params, 1);

    if (connect_result < 0)
    {
        logger.log_error("ScreenCaptureStream", "error connecting to pipewire stream :c", true);
        on_screencast_started.broadcast({format_gerror_string("error connecting to pipewire stream :c", *error), -1});
        return;
    }

    on_screencast_started.broadcast({});
    logger.log("ScreenCaptureStream", "Screencast stream connected! (pipewire + permissions handled!) c:", false);
}

void ScreenCaptureStream::on_stream_state_changed(pw_stream_state old, pw_stream_state state, const char *error)
{
    if (state == PW_STREAM_STATE_ERROR)
    {
        logger.log_error("ScreenCaptureStream", "Status is... error? :c Here's what I've been told... '" + std::string(error) + "'", true);
        return;
    }

    if (state == PW_STREAM_STATE_STREAMING)
    {
        logger.log("ScreenCaptureStream", "Stream status is streaming!", false);
        return;
    }

    if (state == PW_STREAM_STATE_UNCONNECTED)
    {
        //logger.log("ScreenCaptureStream", "Stream status is... unconnected?", true);
        return;
    }

    if (state == PW_STREAM_STATE_CONNECTING)
    {
        //logger.log("ScreenCaptureStream", "Stream status is... connecting...? hopefully without error...", true);
        return;
    }

    if (state == PW_STREAM_STATE_PAUSED)
    {
        //logger.log("ScreenCaptureStream", "Stream status is... paused", true);
        return;
    }
}

void ScreenCaptureStream::on_stream_param_changed(u32 id, const spa_pod *param)
{
    if (param == nullptr)
    {
        logger.log("ScreenCaptureStream", "null param on_stream_param_changed, ignoring!", false);
        return;
    }

    if (id == SPA_PARAM_Format)
    {
        spa_zero(m_spa_video_info);
        spa_format_video_parse(param, &m_spa_video_info);
    }
}

void ScreenCaptureStream::on_process()
{
    pw_buffer* buffer = pw_stream_dequeue_buffer(m_pw_stream);

    spa_video_info_raw raw_info = m_spa_video_info.info.raw;

    screen_buffer.resize(raw_info.size.height * raw_info.size.width);

    if (buffer->buffer->n_datas <= 0)
    {
        logger.log_error("ScreenCaptureStream", "No buffers recieved on_process? :c", true);
        return;
    }
    
    spa_data& fragment_buf = (buffer->buffer->datas[0]);

    if (!fragment_buf.data)
    {
        logger.log_error("ScreenCaptureStream", "Buffer data is null? :c", true);
        return;
    }

    u8* fragment_data = (u8*)fragment_buf.data;

    switch (raw_info.format)
    {
    case SPA_VIDEO_FORMAT_RGB:
        for (usize i = 0; i < screen_buffer.size(); i++)
        {
            screen_buffer[i] = Color(fragment_data[i * 3], fragment_data[i * 3 + 1], fragment_data[i * 3 + 2], 255);
        }
        break;
    
    case SPA_VIDEO_FORMAT_RGBA:
        for (usize i = 0; i < screen_buffer.size(); i++)
        {
            screen_buffer[i] = Color(fragment_data[i * 4], fragment_data[i * 4 + 1], fragment_data[i * 4 + 2], fragment_data[i * 4 + 3]);
        }
        break;

    case SPA_VIDEO_FORMAT_BGRx:
        for (usize i = 0; i < screen_buffer.size(); i++)
        {
            screen_buffer[i] = Color(fragment_data[i * 4 + 2], fragment_data[i * 4 + 1], fragment_data[i * 4], 255);
        }
        break;
    default:
        logger.log_error("ScreenCaptureStream", "Buffer is of unknown format? :c", true);
        return;
    }

    if (buffer == nullptr)
    {
        logger.log_error("ScreenCaptureStream", "Buffer is null? :c", true);
        return;
    }

    on_data_received.broadcast(screen_buffer, raw_info.size.width, raw_info.size.height);

    pw_stream_queue_buffer(m_pw_stream, buffer);
}

void ScreenCaptureStream::static_on_screencast_session_created(GObject* source_object, GAsyncResult* res, void* data)
{
    reinterpret_cast<ScreenCaptureStream*>(data)->on_screencast_session_created(source_object, res);
}

void ScreenCaptureStream::static_on_screencast_session_start(GObject *source_object, GAsyncResult *res, void *data)
{
    reinterpret_cast<ScreenCaptureStream*>(data)->on_screencast_session_start(source_object, res);
}

void ScreenCaptureStream::static_on_stream_state_changed(void *data, pw_stream_state old, pw_stream_state state, const char *error)
{
    reinterpret_cast<ScreenCaptureStream*>(data)->on_stream_state_changed(old, state, error);
}

void ScreenCaptureStream::static_on_stream_param_changed(void *data, u32 id, const spa_pod *param)
{
    reinterpret_cast<ScreenCaptureStream*>(data)->on_stream_param_changed(id, param);
}

void ScreenCaptureStream::static_on_stream_process(void *data)
{
    reinterpret_cast<ScreenCaptureStream*>(data)->on_process();
}

}
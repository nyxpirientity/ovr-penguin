#include "ovrpenguin.hpp"
#include "vr/ovr_runtime.hpp"
#include "io/stdio_handler.hpp"
#include "types/string_command.hpp"
#include "vr/ovr_window_overlay.hpp"
#include "graphics/gl_context.hpp"
#include "media/screen_capturer.hpp"

namespace nyxpiri::ovrpenguin
{
OvrPenguin::OvrPenguin()
{
    io_handler = adopt(Node::construct<StdIoHandler>());
    ovr_runtime = adopt(Node::construct<OvrRuntime>(io_handler));
    gl_context = adopt(Node::construct<GlContext>());
    screen_capturer = adopt(Node::construct<ScreenCapturer, Logger&>(logger));
}

OvrPenguin::~OvrPenguin()
{
}

void OvrPenguin::on_start()
{
    Super::on_start();

    logger.terminal_output_function = [this](const std::string& string){ io_handler->async_print_string(string); };
}

void OvrPenguin::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);

    if (!input_event_binding.is_bound())
    {
        io_handler->async_print_string("> ");
        io_handler->async_await_input().bind(input_event_binding, [this](const std::string& input)
        {
            logger.log("user", "entered: " + input, false);
            execute_command(input);
        });
    }
}

void OvrPenguin::on_stop()
{
    Super::on_stop();
}

void OvrPenguin::execute_command(const std::string& input)
{
    StringCommand command{input};

    if (not command.has_parameter(0))
    {
        logger.log("OvrPenguin", "Blank command? Ignoring (will also flush output buffer)", true);
        return;
    }

    if(command.get_parameter(0) == "stop")
    {
        logger.log("OvrPenguin", "stopping tree", true);
        stop_tree();
    }
    else if (command.get_parameter(0) == "ovr-init")
    {
        logger.log("OvrPenguin", "initializing OpenVR..,,,", true);
        ovr_runtime->initialize();
    }
    else if (command.get_parameter(0) == "new-window-overlay")
    {   
        command.set_options({"--name"});
        
        std::string name = command.get_option_parameter_copy("--name", 0);

        if (name.empty())
        {
            logger.log("OvrPenguin", "new-window-overlay requires --name parameter to denote the overlay name.", true);
            return;
        }

        WeakPtr<OvrWindowOverlay> new_overlay = adopt(Node::construct<OvrWindowOverlay>(gl_context, screen_capturer));
        overlays.push_back(new_overlay);
        new_overlay->set_overlay_name(name);
    }
    else if (command.get_parameter(0) == "list-window-overlays")
    {
        logger.log("OvrPenguin", "Overlay window list!", true);

        for (WeakPtr<OvrWindowOverlay>& overlay : overlays)
        {
            if (overlay == nullptr)
            {
                continue;
            }

            logger.log("OvrPenguin", "- " + overlay->get_overlay_name(), true);
        }
    }
    else if (command.get_parameter(0) == "destroy-window-overlay")
    {
        command.set_options({"--name"});

        std::string name = command.get_option_parameter_copy("--name", 0);

        if (name.empty())
        {
            logger.log("OvrPenguin", "destroy-window-overlay requires --name parameter to denote the overlay name.", true);
            return;
        }

        usize overlay_index = get_overlay_index_by_name(name);

        if (overlay_index == -1)
        {
            logger.log("OvrPenguin", "couldn't find overlay of name '" + name + "' :c", true);
            return;
        }

        overlays[overlay_index]->queue_destroy();
        overlays.erase(overlays.begin() + overlay_index);
        logger.log("OvrPenguin", "destroyed overlay of name '" + name + "'!", true);
    }
    else if (command.get_parameter(0) == "init-window-overlay-capture")
    {
        command.set_options({"--name"});

        std::string name = command.get_option_parameter_copy("--name", 0);

        if (name.empty())
        {
            logger.log("OvrPenguin", "init-window-overlay-capture requires --name parameter to denote the overlay name.", true);
            return;
        }

        WeakPtr<OvrWindowOverlay> overlay = get_overlay_by_name(name);

        if (overlay == nullptr)
        {
            logger.log("OvrPenguin", "couldn't find overlay of name '" + name + "' :c", true);
            return;
        }

        overlay->reset_window_session();
        logger.log("OvrPenguin", "initialized window overlay window capture for '" + name + "'!", true);
    }
    else if (command.get_parameter(0) == "set-window-overlay-type")
    {
        command.set_options({"--name", "--type"});

        std::string name = command.get_option_parameter_copy("--name", 0);

        if (name.empty())
        {
            logger.log("OvrPenguin", "set-window-overlay-type requires --name parameter to denote the overlay name.", true);
            return;
        }

        WeakPtr<OvrWindowOverlay> overlay = get_overlay_by_name(name);

        if (overlay == nullptr)
        {
            logger.log("OvrPenguin", "couldn't find overlay of name '" + name + "' :c", true);
            return;
        }

        std::string type_str = command.get_option_parameter_copy("--type", 0);

        if (type_str.empty())
        {
            logger.log("OvrPenguin", "set-window-overlay-type requires --type parameter to specify the overlay type.", true);
            return;
        }

        if (type_str == "dashboard")
        {
            overlay->set_overlay_type(OvrOverlay::Type::dashboard);
        }
        else
        {
            logger.log("OvrPenguin", "Unsupported type '" + type_str + "'...", true);
        }

        logger.log("OvrPenguin", "set window overlay type to '" + type_str + "' for overlay '" + name + "'!", true);
    }
    else
    {
        logger.log("OvrPenguin", "OvrPenguin: unknown command '" + command.copy_parameter(0) + "'", true);
    }
}

WeakPtr<class OvrWindowOverlay> OvrPenguin::get_overlay_by_name(const std::string &name)
{
    for (WeakPtr<OvrWindowOverlay>& overlay : overlays)
    {
        if (overlay->get_overlay_name() == name)
        {
            return overlay;
        }
    }

    return nullptr;
}

usize OvrPenguin::get_overlay_index_by_name(const std::string &name)
{
    for (usize i = 0; i < overlays.size(); i++)
    {
        if (overlays[i]->get_overlay_name() == name)
        {
            return i;
        }
    }

    return -1;
}

} // namespace nyxpiri::ovrpenguin

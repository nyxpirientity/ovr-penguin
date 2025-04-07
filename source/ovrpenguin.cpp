#include "ovrpenguin.hpp"
#include "vr/ovr_runtime.hpp"
#include "io/stdio_handler.hpp"
#include "types/string_command.hpp"
#include "vr/ovr_overlay.hpp"
#include "graphics/gl_context.hpp"

namespace nyxpiri::ovrpenguin
{
OvrPenguin::OvrPenguin()
{
    io_handler = adopt(Node::construct<StdIoHandler>());
    ovr_runtime = adopt(Node::construct<OvrRuntime>(io_handler));
    gl_context = adopt(Node::construct<GlContext>());
    test_overlay = adopt(Node::construct<OvrOverlay>(gl_context));
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
        test_overlay->set_overlay_type(OvrOverlay::Type::dashboard);
    }
    else
    {
        logger.log("OvrPenguin", "OvrPenguin: unknown command '" + command.copy_parameter(0) + "'", true);
    }
}

} // namespace nyxpiri::ovrpenguin

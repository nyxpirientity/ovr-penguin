#include "ovrpenguin.hpp"
#include "vr/ovr_runtime.hpp"
#include "io/stdio_handler.hpp"
#include "types/string_command.hpp"
#include "vr/ovr_window_overlay.hpp"
#include "graphics/gl_context.hpp"
#include "media/screen_capturer.hpp"
#include "system/filesystem.hpp"

#include <filesystem>
#include <fstream>

namespace nyxpiri::ovrpenguin
{
std::filesystem::path get_aliases_dir()
{
    std::filesystem::path path = filesystem::get_save_dir().value() / "aliases";
    std::filesystem::create_directories(path);
    return path;
}

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
    refresh_aliases();
}

void OvrPenguin::on_tick(real delta_seconds)
{
    Super::on_tick(delta_seconds);

    if (!input_event_binding.is_bound() and wait_for_input)
    {
        io_handler->async_print_string("> ");
        io_handler->async_await_input().bind(input_event_binding, [this](const std::string& input)
        {
            logger.log("OvrPenguin", "user input command '" + input +"'", false);
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
    wait_for_input = true;

    if (not command.has_parameter(0))
    {
        logger.log("OvrPenguin", "Blank command? Ignoring (will also flush output buffer)", true);
        return;
    }

    for (usize i = 0; i < aliases.size(); i++)
    {
        if (aliases[i].first != command.get_parameter(0))
        {
            continue;
        }

        std::string new_input = aliases[i].second;
        new_input.push_back(' ');
        for (usize i = 1; i < command.parameter_count(); i++)
        {
            bool contains_spaces = StringCommand::find_first_delimitter(command.get_parameter(i), 0);

            if (contains_spaces)
            {
                new_input.push_back('\"');
            }

            new_input.append(command.get_parameter(i));

            if (contains_spaces)
            {
                new_input.push_back('\"');
            }

            new_input.push_back(' ');
        }
        
        command.reset(new_input); 
        break;
    }
    

    if(command.get_parameter(0) == "stop")
    {
        logger.log("OvrPenguin", "stopping tree", true);
        stop_tree();
    }
    else if (command.get_parameter(0) == "help")
    {
        logger.log("OvrPenguin", "Available commands!", true);
        logger.log("OvrPenguin", "- help", true);
        logger.log("OvrPenguin", "- stop", true);
        logger.log("OvrPenguin", "- ovr-init", true);
        logger.log("OvrPenguin", "- new-window-overlay --name <name> --type (optional) <type>", true);
        logger.log("OvrPenguin", "- list-window-overlays", true);
        logger.log("OvrPenguin", "- destroy-window-overlay --name <name>", true);
        logger.log("OvrPenguin", "- init-window-overlay-capture --name <name> [--all]", true);
        logger.log("OvrPenguin", "- set-window-overlay-type --name <name> --type <type>", true);
        logger.log("OvrPenguin", "- set-window-overlay-properties --size <size> --curve <curvature (0.0 - 1.0)>", true);
        logger.log("OvrPenguin", "- exec --file <file>", true);
    }
    else if (command.get_parameter(0) == "ovr-init")
    {
        logger.log("OvrPenguin", "initializing OpenVR..,,,", true);
        ovr_runtime->initialize();
    }
    else if (command.get_parameter(0) == "new-window-overlay")
    {   
        if (not ovr_runtime->is_running())
        {
            logger.log("OvrPenguin", "Cannot create a new overlay due to OpenVR not being initialized :c (you may need to call ovr-init still?)", true);
            return;
        }

        command.set_options({"--name", "--type"});
        
        std::string name = command.get_option_parameter_copy("--name", 0);

        if (name.empty())
        {
            logger.log("OvrPenguin", "new-window-overlay requires --name parameter to denote the overlay name.", true);
            return;
        }

        WeakPtr<OvrWindowOverlay> new_overlay = adopt(Node::construct<OvrWindowOverlay>(gl_context, screen_capturer));
        overlays.push_back(new_overlay);
        new_overlay->set_overlay_name(name);

        std::string type_str = command.get_option_parameter_copy("--type", 0);

        if (!type_str.empty())
        {
            set_overlay_type(new_overlay, type_str);
        }
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
        bool all = command.has_parameter("--all") and name.empty();

        if (name.empty() and not all)
        {
            logger.log(
                "OvrPenguin", 
                "init-window-overlay-capture requires --name parameter to denote the overlay name. "
                "Alternatively, you can use --all to initialize all overlays.", true);
            return;
        }

        if (all)
        {
            wait_for_input = false;
            current_window_overlay_capture_init_index = 0;
            init_next_overlay_capture();
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

        set_overlay_type(overlay, type_str);
    }
    else if (command.get_parameter(0) == "set-window-overlay-properties")
    {
        command.set_options({"--name", "--size", "--curve"});

        std::string name = command.get_option_parameter_copy("--name", 0);

        if (name.empty())
        {
            logger.log("OvrPenguin", "resize-window-overlay requires --name parameter to denote the overlay name.", true);
            return;
        }

        WeakPtr<OvrWindowOverlay> overlay = get_overlay_by_name(name);

        if (overlay == nullptr)
        {
            logger.log("OvrPenguin", "couldn't find overlay of name '" + name + "' :c", true);
            return;
        }

        std::string size_str = command.get_option_parameter_copy("--size", 0);

        if (!size_str.empty())
        {
            f64 size = std::stod(size_str);
            overlay->set_size(size);
        }

        std::string curve_str = command.get_option_parameter_copy("--curve", 0);

        if (!curve_str.empty())
        {
            f64 curve = std::stod(curve_str);
            overlay->set_curve(curve);
        }
    }
    else if (command.get_parameter(0) == "refresh-aliases")
    {
        refresh_aliases();
    }
    else if (command.get_parameter(0) == "list-aliases")
    {
        logger.log("OvrPenguin", "currently loaded alias list!", true);
        
        for (const auto& alias : aliases)
        {
            logger.log("OvrPenguin", "- " + alias.first + " => " + alias.second, true);
        }
    }
    else if (command.get_parameter(0) == "new-alias")
    {
        command.set_options({"--file", "--name", "--value"});

        std::string name = command.get_option_parameter_copy("--name", 0);
        std::string value = command.get_option_parameter_copy("--value", 0);

        if (name.empty() or value.empty())
        {
            logger.log("OvrPenguin", "new-alias requires --name and --value parameter to specify the name and value of the alias :c", true);
            return;
        }

        std::filesystem::path file_path = command.get_option_parameter_copy("--file", 0);

        if (file_path.empty())
        {
            file_path = get_aliases_dir() / "default";
        }
        else
        {
            if (!file_path.is_absolute())
            {
                file_path = get_aliases_dir() / file_path;
            }
        }
        
        bool making_new_file = not std::filesystem::exists(file_path);

        if (not making_new_file and std::filesystem::is_directory(file_path))
        {
            logger.log("OvrPenguin", "Couldn't write new alias to " + file_path.string() + ", as it seemed to be a directory :c", true);
            return;
        }

        std::ofstream output_stream{file_path, std::ios_base::app};

        output_stream.write("ovr-penguin-aliases!~\n", 22);
        std::string alias_string = ("\"" + name + "\" \"" + value + "\"");
        output_stream.write(alias_string.c_str(), alias_string.size());
        
        output_stream.close();
    }
    else if (command.get_parameter(0) == "exec")
    {
        command.set_options({"--file"});

        std::string file = command.get_option_parameter_copy("--file", 0);

        if (file.empty())
        {
            logger.log("OvrPenguin", "exec requires --file parameter to denote the file to execute.", true);
            return;
        }
        std::filesystem::path file_path(file);
        file_path = std::filesystem::absolute(file_path);
        std::ifstream file_stream(file_path);
        
        if (!std::filesystem::exists(file_path))
        {
            logger.log("OvrPenguin", "couldn't find file '" + file_path.string() + "' :c", true);
            return;
        }

        if (!file_stream.is_open())
        {
            logger.log("OvrPenguin", "couldn't open file '" + file_path.string() + "' :c", true);
            return;
        }

        std::string line;

        std::getline(file_stream, line);
        
        if (line != "ovr-penguin-executable!~")
        {
            logger.log("OvrPenguin", "file '" + file + "' is not a valid ovr-penguin executable file... (first line must be 'ovr-penguin-executable!~')", true);
            return;
        }

        while (std::getline(file_stream, line))
        {
            if (line.empty())
            {
                continue;
            }

            io_handler->async_print_string("> " + line + "\n");
            logger.log("OvrPenguin", "exec executing command: '" + line + "'", false);
            execute_command(line);
        }

        file_stream.close();
    }
    else if (command.get_parameter(0).find_first_of("//") == 0)
    {
        logger.log("OvrPenguin", "ignoring comment command: '" + command.get_raw_command() + "'", false);
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

void OvrPenguin::init_overlay_capture(WeakPtr<class OvrWindowOverlay> overlay)
{
    overlay->on_window_session_started.bind(init_window_overlay_capture_log_binding, [this, overlay](Result<> result)
    {
        if (result.is_error())
        {
            logger.log("OvrPenguin", "failed to initialize window overlay window capture for '" + overlay->get_overlay_name() + "' :c", true);
            return;
        }

        logger.log("OvrPenguin", "initialized window overlay window capture for '" + overlay->get_overlay_name() + "'! c:", true);
    });
    
    overlay->reset_window_session();
}

void OvrPenguin::set_overlay_type(WeakPtr<class OvrWindowOverlay> overlay, const std::string &type_str)
{
    if (type_str == "dashboard")
    {
        overlay->set_overlay_type(OvrOverlay::Type::dashboard);
    }
    else if (type_str == "world")
    {
        overlay->set_overlay_type(OvrOverlay::Type::world);
    }
    else
    {
        logger.log("OvrPenguin", "Unsupported type '" + type_str + "'...", true);
    }

    logger.log("OvrPenguin", "set window overlay type to '" + type_str + "' for overlay '" + overlay->get_overlay_name() + "'!", true);
}

void OvrPenguin::refresh_aliases()
{
    std::filesystem::recursive_directory_iterator iterator{get_aliases_dir()};
    
    aliases.clear();
    aliases.reserve(32);
    logger.log("OvrPenguin", "Scanning for alias files in path '" + get_aliases_dir().string() + "'", false);
    std::ifstream file_input_stream;
    std::string file_input;
    for (const std::filesystem::directory_entry& entry : iterator)
    {
        if (!entry.is_regular_file())
        {
            logger.log("OvrPenguin", "aliases directory entry '" + entry.path().string() + "' is_regular_file() returned false, ignoring...", false);
            continue;
        }

        file_input_stream.open(entry.path());
        std::getline(file_input_stream, file_input);
        
        if (file_input != "ovr-penguin-aliases!~")
        {
            logger.log("OvrPenguin", "aliases directory entry '" + entry.path().string() + "' ignored due to 'ovr-penguin-aliases!~' not being the first line", true);
            continue;
        }

        while (std::getline(file_input_stream, file_input))
        {
            if (file_input.empty())
            {
                continue;
            }

            StringCommand line_command = file_input;
            
            if (line_command.has_parameter(0))
            {
                if (line_command.get_parameter(0).find_first_of("//") == 0)
                {
                    continue;
                }
            }

            if (!line_command.has_parameter(1))
            {
                logger.log("OvrPenguin", "aliases file entry '" + file_input + "' in file '" + entry.path().string() + "' did not seem to be complete, stopping parse", true);
                break;
            }

            std::string alias_value;
            
            for (usize i = 1; i < line_command.parameter_count(); i++)
            {
                bool contains_spaces = StringCommand::find_first_delimitter(line_command.get_parameter(i), 0);

                if (contains_spaces)
                {
                    alias_value.push_back('\"');
                }
                
                alias_value.append(line_command.get_parameter(i));
                
                if (contains_spaces)
                {
                    alias_value.push_back('\"');
                }

                alias_value.push_back(' ');
            }
            

            aliases.push_back({line_command.copy_parameter(0), alias_value});
        }

        file_input_stream.close();
    }
}

void OvrPenguin::init_next_overlay_capture()
{
    if (current_window_overlay_capture_init_index >= overlays.size())
    {
        logger.log("OvrPenguin", "finished initializing all overlays window capturing! c:", true);
        wait_for_input = true;
        current_window_overlay_capture_init_index = -1;
        return;
    }

    WeakPtr<OvrWindowOverlay> overlay = overlays[current_window_overlay_capture_init_index];
    current_window_overlay_capture_init_index++;

    overlay->on_window_session_started.bind(init_all_window_overlay_capture_binding, [this](Result<> result)
    {
        init_next_overlay_capture();
    });

    logger.log("OvrPenguin", "initializing window overlay window capture for '" + overlay->get_overlay_name() + "'...", true);

    init_overlay_capture(overlay);
}

} // namespace nyxpiri::ovrpenguin

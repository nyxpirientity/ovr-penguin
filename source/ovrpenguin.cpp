#include "ovrpenguin.hpp"
#include "vr/ovr_runtime.hpp"
#include "io/stdio_handler.hpp"
#include "types/string_command.hpp"
#include "vr/ovr_window_overlay.hpp"
#include "graphics/gl_context.hpp"
#include "media/screen_capturer.hpp"
#include "system/filesystem.hpp"
#include <openvr.h>

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

std::filesystem::path get_executables_dir()
{
    std::filesystem::path path = filesystem::get_save_dir().value() / "executables";
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
    
    /* assumes --name option is already set */
    auto try_get_overlay_by_name_param_required = [this, &command](const std::string& cmd_name)
    {
        std::string name = command.get_option_parameter_copy("--name", 0);

        if (name.empty())
        {
            logger.log("OvrPenguin", cmd_name + " requires --name parameter to specify the overlay name..", true);
            return WeakPtr<OvrWindowOverlay>{nullptr};
        }

        WeakPtr<OvrWindowOverlay> overlay = get_overlay_by_name(name);

        if (!overlay)
        {
            logger.log("OvrPenguin", "couldn't find overlay of name '" + name + "' :c", true);
            return WeakPtr<OvrWindowOverlay>{nullptr};
        }

        return overlay; 
    };

    /* assumes --index is already set */
    auto get_and_verify_color_key_index_by_name_param_required = [this, &command](const std::string& cmd_name, WeakPtr<OvrWindowOverlay> overlay)
    {
        std::string index_str = command.get_option_parameter_copy("--index", 0);

        if (index_str.empty())
        {
            logger.log("OvrPenguin", cmd_name + " requires --index parameter to specify the colorkey index..", true);
            return usize(-1);
        }
        
        usize index = std::stoull(index_str);

        if (index >= overlay->get_num_color_keys())
        {
            logger.log("OvrPenguin", "colorkey index doesn't seem to exist yet :c", true);
            return usize(-1);
        }

        return index;
    };

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
        io_handler->async_print_string("Available commands!\n");
        io_handler->async_print_string(
            "- help\n"
            "    outputs this message\n\n");
        
        io_handler->async_print_string(
            "- stop\n"
            "    stops the node tree, and exiting the program\n\n"
        );
    
        io_handler->async_print_string(
            "- ovr-init\n"
            "    initializes OpenVR (if not already initialized)\n\n"
        );
    
        io_handler->async_print_string(
            "- new-window-overlay --name <string> [--type <type>]\n"
            "    creates a new window overlay with the given name and type. The type can be 'dashboard' or 'world'.\n\n"
        );
    
        io_handler->async_print_string(
            "- list-window-overlays\n"
            "    lists all window overlays that have been created.\n\n"
        );
    
        io_handler->async_print_string(
            "- destroy-window-overlay --name <string>\n"
            "    destroys the overlay with the given name.\n\n"
        );
    
        io_handler->async_print_string(
            "- init-window-overlay-capture --name <string> [--all]\n"
            "    initializes the window capture for the overlay with the given name. If --all is specified, all overlays will be initialized.\n\n"
        );
    
        io_handler->async_print_string(
            "- set-window-overlay-type --name <string> --type <type>\n"
            "    sets the type of the overlay with the given name to the given type. The type can be 'dashboard' or 'world'.\n\n"
        );
    
        io_handler->async_print_string(
            "- set-window-overlay-properties --name <string> [--size <number>] [--curve <number>] [--parent <parent>] [--position <vector3>] [--rotation <vector3>] [--hidden <boolean>] [--alpha <number>] [--top-crop <number>] [--bottom-crop <number>] [--right-crop <number>] [--left-crop <number>]\n"
            "    sets the properties of the overlay with the given name. \n"
            "    --size <number> ~ sets the size of the overlay (default: 1.0)\n"
            "    --curve <number> ~ sets the curve of the overlay (default: 0.0)\n"
            "    --parent <origin/left-hand/right-hand/hmd> ~ sets the parent of the overlay (default: origin)\n"
            "    --position <vector3> ~ sets the position of the overlay (default: 0.0, 0.0, 0.0)\n"
            "    --rotation <vector3> ~ sets the rotation of the overlay (default: 0.0, 0.0, 0.0)\n"
            "    --hidden <boolean> ~ sets whether the overlay is hidden (default: false)\n"
            "    --alpha <number> ~ sets the alpha of the overlay (default: 1.0)\n"
            "    --top-crop <number> ~ sets the top crop of the overlay (default: 0)\n"
            "    --bottom-crop <number> ~ sets the bottom crop of the overlay (default: 0)\n"
            "    --right-crop <number> ~ sets the right crop of the overlay (default: 0)\n"
            "    --left-crop <number> ~ sets the left crop of the overlay (default: 0)\n\n"
        );
        
        io_handler->async_print_string(
            "- new-window-overlay-color-key --name <string>\n"
            "    creates a new color key for overlay of name, outputs back the index (like an id number) of the new color key\n\n"
        );

        io_handler->async_print_string(
            "- set-window-overlay-color-key-properties --name <string> --index <integer> [--color <vector3>] [--min <number>] [--max <number>]\n"
            "    sets the properties of the color key with the given name and index. \n"
            "    --color <vector3> ~ sets the color of the color key (default: 0.0, 0.0, 0.0)\n"
            "    --min <number> ~ sets the min range/distance of the color key (default: 0.01)\n"
            "    --max <number> ~ sets the max range/distance of the color key (default: 0.02)\n\n"
        );

        io_handler->async_print_string(
            "- destroy-window-overlay-color-key --name <string> --index <integer>\n"
            "    destroys the color key with the given name and index.\n\n"
        );
    
        io_handler->async_print_string(
            "- list-window-overlay-color-keys --name <string>\n"
            "    lists all color keys for overlay of name\n\n"
        );

        io_handler->async_print_string(
            "- refresh-aliases\n"
            "    refreshes the aliases from disk\n\n"
        );
    
        io_handler->async_print_string(
            "- list-aliases\n"
            "    lists all aliases currently loaded\n\n"
        );
    
        io_handler->async_print_string(
            "- new-alias --file <path> --name <string> --value <value>\n"
            "    creates a new alias with the given name and value, and writes it to the given file.\n\n"
        );

        io_handler->async_print_string(
            "- exec --file <path>\n"
            "    executes the given OvrPenguin executable file (a series of commands) with the given name.\n\n"
        );
        
        io_handler->async_print_string(
            "- save-exec --name <string> --file <path> [--dupe] [--overwrite]\n"
            "    serializes the current state as an executable that can be executed with exec, and saves it to the given file.\n"
            "    The file will be created if it doesn't exist.\n"
            "    --dupe ~ duplicates the file to a new name before writing, if a file of the given path already exists.\n"
            "    --overwrite ~ overwrites the file if it already exists.\n\n");
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

        for (WeakPtr<OvrWindowOverlay> overlay : overlays)
        {
            if (overlay->get_overlay_name() == name)
            {
                logger.log("OvrPenguin", "overlay names must be unique...", true);
                return;
            }
        }

        const usize max_overlay_name_length = (std::min(vr::k_unVROverlayMaxNameLength, vr::k_unVROverlayMaxKeyLength) - 1);

        for (WeakPtr<OvrWindowOverlay> overlay : overlays)
        {
            if (overlay->get_overlay_name().size() > max_overlay_name_length)
            {
                logger.log("OvrPenguin", "overlay names cannot be more than " + std::to_string(max_overlay_name_length) + " c chars... (openvr limitation, okay?)", true);
                return;
            }
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
            logger.log("OvrPenguin", "destroy-window-overlay requires --name parameter to specify the overlay name.", true);
            return;
        }

        usize overlay_index = get_overlay_index_by_name(name);

        if (overlay_index == -1)
        {
            logger.log("OvrPenguin", "couldn't find overlay of name '" + name + "' :c", true);
            return;
        }

        destroy_overlay(overlay_index);
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

        WeakPtr<OvrWindowOverlay> overlay = try_get_overlay_by_name_param_required("set-window-overlay-type");

        if (!overlay)
        {
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
        command.set_options({"--name", "--size", "--curve", "--parent", "--position", "--rotation", "--hidden", "--alpha",
            "--top-crop", "--bottom-crop", "--right-crop", "--left-crop"});

        WeakPtr<OvrWindowOverlay> overlay = try_get_overlay_by_name_param_required("set-window-overlay-properties");

        if (!overlay)
        {
            return;
        }

        run_if_cmd_option_real(command, "--size", 0, [this, overlay](real val)
        {
            overlay->set_size(val);
        });

        run_if_cmd_option_real(command, "--curve", 0, [this, overlay](real val)
        {
            overlay->set_curve(val);
        });

        run_if_cmd_option_real(command, "--alpha", 0, [this, overlay](real val)
        {
            overlay->set_alpha(val);
        });

        run_if_cmd_option_usize(command, "--top-crop", 0, [this, overlay](usize val)
        {
            overlay->set_top_crop(val);
        });

        run_if_cmd_option_usize(command, "--bottom-crop", 0, [this, overlay](usize val)
        {
            overlay->set_bottom_crop(val);
        });

        run_if_cmd_option_usize(command, "--right-crop", 0, [this, overlay](usize val)
        {
            overlay->set_right_crop(val);
        });
        
        run_if_cmd_option_usize(command, "--left-crop", 0, [this, overlay](usize val)
        {
            overlay->set_left_crop(val);
        });

        std::string parent_str = command.get_option_parameter_copy("--parent", 0);

        if (!parent_str.empty())
        {
            if (parent_str == "left-hand")
            {
                overlay->set_overlay_parent(OverlayParent::LeftMotionController);
            }
            else if (parent_str == "right-hand")
            {
                overlay->set_overlay_parent(OverlayParent::RightMotionController);
            }
            else if (parent_str == "hmd" or parent_str == "headset")
            {
                overlay->set_overlay_parent(OverlayParent::HeadMountedDisplay);
            }
            else if (parent_str == "origin")
            {
                overlay->set_overlay_parent(OverlayParent::PlaySpace);
            }
        }

        run_if_cmd_option_vec3(command, "--position", 0, [this, overlay](Vec3 val)
        {
            overlay->set_overlay_position(val);
        });

        run_if_cmd_option_vec3(command, "--rotation", 0, [this, overlay](Vec3 val)
        {
            overlay->set_overlay_rotation(val);
        });


        std::string hidden_str = command.get_option_parameter_copy("--hidden", 0);
        
        if (!hidden_str.empty())
        {
            if (hidden_str == "true")
            {
                overlay->user_hide_overlay();
            }
            else if (hidden_str == "false")
            {
                overlay->user_show_overlay();
            }
            else
            {
                logger.log("OvrPenguin", "set-window-overlay-properties --hidden expects 'true' or 'false' value", true);
            }
        }
    }
    else if (command.get_parameter(0) == "list-window-overlay-color-keys")
    {
        command.set_options({"--name"});

        WeakPtr<OvrWindowOverlay> overlay = try_get_overlay_by_name_param_required("list-window-overlay-color-keys");
        
        if (!overlay)
        {
            return;
        }

        logger.log("OvrPenguin", "color key list!", true);
        for (usize i = 0; i < overlay->get_num_color_keys(); i++)
        {
            logger.log("OvrPenguin", "- " + std::to_string(i) + " => " + overlay->get_color_key_string(i), true);
        }
    }
    else if (command.get_parameter(0) == "new-window-overlay-color-key")
    {
        command.set_options({"--name"});
        
        WeakPtr<OvrWindowOverlay> overlay = try_get_overlay_by_name_param_required("new-window-overlay-color-key");
        
        if (!overlay)
        {
            return;
        }

        usize index = overlay->new_color_key();
        
        logger.log("OvrPenguin", "new color key created for overlay at index " + std::to_string(index), true);
    }
    else if (command.get_parameter(0) == "set-window-overlay-color-key-properties")
    {
        command.set_options({"--name", "--index", "--color", "--min", "--max"});
        WeakPtr<OvrWindowOverlay> overlay = try_get_overlay_by_name_param_required("set-window-overlay-color-key-properties");
        
        if (!overlay)
        {
            return;
        }

        usize index = get_and_verify_color_key_index_by_name_param_required("set-window-overlay-color-key-properties", overlay);
        
        if (index == -1)
        {
            return;
        }

        run_if_cmd_option_vec3(command, "--color", 0, [this, overlay, index](Vec3 val)
        {
            overlay->set_color_key_color(index, Color{val.x, val.y, val.z, 1.0});
        });

        run_if_cmd_option_real(command, "--min", 0, [this, overlay, index](real val)
        {
            overlay->set_color_key_min(index, val);
        });
        
        run_if_cmd_option_real(command, "--max", 0, [this, overlay, index](real val)
        {
            overlay->set_color_key_max(index, val);
        });
    }
    else if (command.get_parameter(0) == "destroy-window-overlay-color-key")
    {
        command.set_options({"--name", "--index"});
        WeakPtr<OvrWindowOverlay> overlay = try_get_overlay_by_name_param_required("destroy-window-overlay-color-key");
        
        if (!overlay)
        {
            return;
        }

        usize index = get_and_verify_color_key_index_by_name_param_required("destroy-window-overlay-color-key", overlay);
        
        if (index == -1)
        {
            return;
        }

        overlay->destroy_color_key(index);
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

        if (not output_stream.is_open())
        {
            logger.log_error("OvrPenguin", "Couldn't open stream? :c", true);
            return;
        }

        output_stream.write("ovr-penguin-aliases!~\n", 22);
        std::string alias_string = ("\"" + name + "\" \"" + value + "\"");
        output_stream.write(alias_string.c_str(), alias_string.size());
        
        output_stream.close();
    }
    else if (command.get_parameter(0) == "exec")
    {
        command.set_options({"--file"});

        std::filesystem::path file_path(command.get_option_parameter_copy("--file", 0));

        if (file_path.empty())
        {
            logger.log("OvrPenguin", "exec requires --file parameter to denote the file to execute.", true);
            return;
        }

        if (not file_path.is_absolute())
        {
            file_path = get_executables_dir() / file_path;
        }
        
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
            logger.log("OvrPenguin", "file '" + file_path.string() + "' is not a valid ovr-penguin executable file... (first line must be 'ovr-penguin-executable!~')", true);
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
    else if (command.get_parameter(0) == "save-exec")
    {
        command.set_options({"--file", "--overwrite", "--dupe"});

        std::filesystem::path file = command.get_option_parameter("--file", 0);

        if (file.empty())
        {
            logger.log("OvrPenguin", "save-exec requires --file option to specify what file to save to", true);
            return;
        }

        if (not file.is_absolute())
        {
            file = get_executables_dir() / file;
        }

        const bool overwrite = command.has_parameter("--overwrite");
        const bool dupe = command.has_parameter("--dupe");
        const bool exists = std::filesystem::exists(file);

        if (exists and not overwrite and not dupe)
        {
            logger.log("OvrPenguin", "by default, save-exec will not overwrite files. to change this, add --overwrite option to the command. alternatively, use --dupe to copy old file data to a new file before saving", true);
            return;
        }

        if (exists and not std::filesystem::is_regular_file(file))
        {
            logger.log("OvrPenguin", "file seems to exist, but is not a regular file (might be a directory, or symlink, etc?), not saving", true);
            return;
        }

        if (dupe and exists)
        {
            bool safe_path = false;
            usize path_num = 0;
            std::filesystem::path new_path = file;
            
            while (not safe_path)
            {
                new_path.replace_filename(file.filename().string() + "_copy-" + std::to_string(path_num));
                safe_path = not std::filesystem::exists(new_path);
                path_num += 1;
            }

            logger.log("OvrPenguin", "copying " + file.string() + " to " + new_path.string(), true);

            std::filesystem::copy_file(file, new_path);
        }

        std::ofstream output_stream{ file };

        if (not output_stream.is_open())
        {
            logger.log_error("OvrPenguin", "couldn't open stream? :c", true);
            return;
        }

        std::string serialized_state = serialize_state_to_exec();
        output_stream.write(serialized_state.c_str(), serialized_state.size());
        logger.log("OvrPenguin", "saved to " + file.string(), true);
    }
    else if (command.get_parameter(0).find_first_of("//") == 0)
    {
        logger.log("OvrPenguin", "ignoring comment command: '" + command.get_raw_command() + "'", false);
    }
    else if (command.get_parameter(0) == "reset")
    {
        logger.log("OvrPenguin", "resetting state!", true);
        reset_state();
        logger.log("OvrPenguin", "finished resetting state!", true);
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

        if (not file_input_stream.is_open())
        {
            logger.log_error("OvrPenguin", "Couldn't open stream when refreshing aliases for file " + entry.path().string() + "? :c", true);
            continue;
        }

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

std::string OvrPenguin::serialize_state_to_exec()
{
    std::string output = "ovr-penguin-executable!~\novr-init\n";

    for (WeakPtr<OvrWindowOverlay> overlay : overlays)
    {
        std::string type_string;
        const std::string& overlay_name = overlay->get_overlay_name();
        const Vec3& ol_pos = overlay->get_overlay_position();
        const Vec3& ol_rot = overlay->get_overlay_rotation();
        f64 ol_curve = overlay->get_curve();
        f64 ol_size = overlay->get_size();
        real ol_alpha = overlay->get_alpha();
        
        real top_crop = overlay->get_top_crop();
        real bottom_crop = overlay->get_bottom_crop();
        real right_crop = overlay->get_right_crop();
        real left_crop = overlay->get_left_crop();

        std::string parent_string;
        std::string ol_hidden_str;


        switch (overlay->get_overlay_type())
        {
        case OvrOverlay::Type::dashboard:
            type_string = "dashboard";
            break;
        case OvrOverlay::Type::world:
            type_string = "world";
            break;
        case OvrOverlay::Type::null:
            continue;
        }

        switch (overlay->get_overlay_parent())
        {
            case OverlayParent::HeadMountedDisplay:
                parent_string = "hmd";
                break;
            case OverlayParent::LeftMotionController:
                parent_string = "left-hand";
                break;
            case OverlayParent::RightMotionController:
                parent_string = "right-hand";
                break;
            case OverlayParent::PlaySpace:
                parent_string = "origin";
                break;
        }

        if (overlay->is_hid_by_user())
        {
            ol_hidden_str = "true";
        }
        else
        {
            ol_hidden_str = "false";
        }

        output.append(
            "new-window-overlay --name \"" + overlay_name + "\" --type " + type_string + "\n"
            "set-window-overlay-properties --name \"" + overlay_name + "\" "
                "--position " + std::to_string(ol_pos.x) + " " + std::to_string(ol_pos.y) + " " + std::to_string(ol_pos.z) + " "
                "--rotation " + std::to_string(ol_rot.x) + " " + std::to_string(ol_rot.y) + " " + std::to_string(ol_rot.z) + " "
                "--curve " + std::to_string(ol_curve) + " "
                "--size " + std::to_string(ol_size) + " "
                "--alpha " + std::to_string(ol_alpha) + " "
                "--top-crop " + std::to_string(top_crop) + " "
                "--bottom-crop " + std::to_string(bottom_crop) + " "
                "--right-crop " + std::to_string(right_crop) + " "
                "--left-crop " + std::to_string(left_crop) + " "
                "--hidden " + ol_hidden_str + " "
                "--parent " + parent_string + "\n"
        );

        for (usize i = 0; i < overlay->get_num_color_keys(); i++)
        {
            output.append(
                "new-window-overlay-color-key --name \"" + overlay_name + "\"\n"
                "set-window-overlay-color-key-properties --name \"" + overlay_name + "\" --index " + std::to_string(i) + " " + overlay->get_color_key_string(i) + "\n"
            );
        }
    }

    return output;
}

void OvrPenguin::destroy_overlay(usize overlay_index)
{
    overlays[overlay_index]->queue_destroy();
    overlays.erase(overlays.begin() + overlay_index);
}

void OvrPenguin::reset_state()
{
    while (overlays.size() > 0)
    {
        destroy_overlay(overlays.size() - 1);
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

void OvrPenguin::run_if_cmd_option_vec3(const StringCommand &command, const std::string &option, usize from_index, std::function<void(Vec3 val)> function)
{
    std::string x_str = command.get_option_parameter_copy(option, from_index);
    std::string y_str = command.get_option_parameter_copy(option, from_index + 1);
    std::string z_str = command.get_option_parameter_copy(option, from_index + 2);

    if (!x_str.empty() and !y_str.empty() and !z_str.empty())
    {
        real x = std::stod(x_str);
        real y = std::stod(y_str);
        real z = std::stod(z_str);

        function({x, y, z});
    }
}

void OvrPenguin::run_if_cmd_option_real(const StringCommand &command, const std::string &option, usize from_index, std::function<void(real val)> function)
{
    std::string param = command.get_option_parameter_copy(option, from_index);

    if (!param.empty())
    {
        real val = std::stod(param);

        function(val);
    }
}

void OvrPenguin::run_if_cmd_option_usize(const StringCommand &command, const std::string &option, usize from_index, std::function<void(usize val)> function)
{
    std::string param = command.get_option_parameter_copy(option, from_index);

    if (!param.empty())
    {
        usize val = std::stoull(param);

        function(val);
    }
}

} // namespace nyxpiri::ovrpenguin

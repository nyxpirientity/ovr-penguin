#include "io/stdio_handler.hpp"
#include "scene/root_node.hpp"
#include "types/mutex.hpp"
#include "vr/ovr_runtime.hpp"
#include "vr/xrdevice.hpp"
#include "ovrpenguin.hpp"
#include <iostream>
#include "types/string_command.hpp"
#include <types/event.hpp>
#include <pipewire/pipewire.h>
#include <GLFW/glfw3.h>


using namespace nyxpiri::ovrpenguin;

/*static void on_screencast_session_start(GObject* source_object, GAsyncResult* res, gpointer data)
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

static void on_screencast_session_creation(GObject* source_object, GAsyncResult* res, gpointer data)
{
    GLibErrorPtr error;
    GLibAutoPtr<XdpSession> session = xdp_portal_create_screencast_session_finish(XDP_PORTAL(source_object), res, error.pass());

    if (error)
    {
        std::cerr << format_gerror_string("Screencast XdpSession creation failed :c", *error);
        return;
    }

    std::cout << "Starting XdpSession!\n";

    xdp_session_start(session.get(), NULL, NULL, on_screencast_session_start, NULL);
}
*/

void opengl_debug_callback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam)
    {
        return;
    }

int main(int argc, char** argv)
{
    // Why ???
    std::cout << "Crowbars are very versatile.\n";
    std::cout << "Penguins have been neglected.\n";
    std::cout << "Time to stand up for the birds.\n";
    std::cout << "And use an electronic brick with an interactive stereoscopic motion picture display.\n\n";
    std::cout << "           _---_        \n";
    std::cout << "          / o - \\       \n";
    std::cout << "          |  V  |       \n";
    std::cout << "         / /   \\ \\      \n";
    std::cout << "        | /     \\ |     \n";
    std::cout << "       | |       | |    \n";
    std::cout << "       | |       | |    \n";
    std::cout << "       \\ /\\/\\ /\\/\\ /    \n";
    std::cout << "        -\\__/-\\__/-     \n";
    std::cout << "       OVR Penguin\n";
    
    std::cout << "Initializing GLFW...\n";
    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "GLFW initialization failed!\nExiting... :c\n";
        return 0;
    }
    std::cout << "GLFW initialized with version '" << glfwGetVersionString() << "'!\n\n";

    std::cout << "Initializing Pipewire...\n";
    pw_init(&argc, &argv);
    std::cout << "Pipewire initialized with version '" << pw_get_library_version() << "'!\n\n";
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "Welcome to OVR Penguin, a CLI utility for reducing the shortcomings of SteamVR on Linux Wayland systems!\n"
                 "Input 'help' for guidance on how to use OVR Penguin.\n"
                 "OVR Penguin uses profiles, which are to save and load setups/states. By default, you are loaded into an empty profile.\n"
                 "Due to SteamVR printing to standard out automatically, OVRPenguin will not initialize/connect to OpenVR until you enter the command 'ovr-init'.\n";
    
    RootNode* root_node = new RootNode();
    root_node->adopt(Node::construct<OvrPenguin>());

    root_node->start();

    delete root_node;

    std::cout << "\n----------------------\nFinished execution! Returning 0! c:\n";
    
    return 0;
}

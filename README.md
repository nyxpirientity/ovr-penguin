# ovr-penguin
## A CLI program for resolving some SteamVR limitations or bugs on Linux Wayland systems. 
Currently, ovr-penguin only serves to give users the ability to setup window capture overlays, to be able to see and view their desktop or individual windows in VR. There is a chance of additional features being added to assist in other limitations of SteamVR.

## **ovr-penguin *exclusively* supports linux**

## Notes
ovr-penguin was **not** made to be a perfectly polished peice of software, it was made for personal use to resolve problems I had when switching to Linux.
It was tested on only _one_ system, running Fedora KDE Plasma, RTX 3070, with an AMD processor, on a Valve Index.
Additionally, as much as I believe this need not to be said, ovr-penguin is not affiliated to, endorsed by or in any way associated to OpenVR or Valve Corporation. It's an independent mess of a tool. 

## General Features
Due to being a CLI software, especially one for VR use, there was some effort put into allowing reduction of verbosity and giving some convenience to avoid users typing too many commands everytime during use.
- basic command macro/executable support, allowing basic pre-made series of commands to be input by reading from a file.
- saving entire state to executable file/macro file (not a literal Linux executable)
- command aliases
- an internal structure that could use lots of improvement

## Window Overlay Capture Features
- World or Dashboard overlays
- Curved overlays, alpha adjustment, and attachment to left-hand, right-hand or hmd/headset
- color keying
- cropping

# Building/Compiling
## Dependencies
Like any C/C++ program, ovr-penguin tries to avoid dependencies, but it still uses a number of libraries.
- [libportal](https://github.com/flatpak/libportal) ~ for requesting screencast permissions (or whatever other permissions must be requested via xdg-desktop-portal)
- [pipewire](https://pipewire.org/) ~ for receiving locally streamed data, such as screencast data
- [glad](https://github.com/Dav1dde/glad)/[opengl](https://www.khronos.org/opengl/) ~ currently used for uploading texture data to the gpu for OpenVR to read, generated glad files are included in the repository.
- [glfw](https://github.com/glfw/glfw) ~ for opening a OpenGL context
- [openvr](https://github.com/ValveSoftware/openvr) ~ this is an OpenVR overlay, of course this depends on it

## Build System
ovr-penguin uses CMake for building, currently only presets for Clang have been created. No extra steps beyond basic CMake usage should be necessary.

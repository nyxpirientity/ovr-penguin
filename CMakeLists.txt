cmake_minimum_required(VERSION 3.30)
set(PROJECT_NAME ovrpenguin)
project(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)

add_executable(${PROJECT_NAME}
    source/main.cpp
    source/nyxpiri_glib_abst/glib_auto_ptr.hpp
    source/nyxpiri_glib_abst/glib_print_utils.hpp
    source/nyxpiri_glib_abst/glib_error_ptr.hpp
    source/scene/node.hpp source/scene/node.cpp
    source/math/vec3.hpp
    source/math/num_types.hpp
    source/math/vec2.hpp
    source/scene/node3d.hpp source/scene/node3d.cpp
    source/math/mat4x4.hpp
    source/math/vec4.hpp

    source/nyxmemory.hpp
    source/types/nyxarrays.hpp
    source/vr/xrdevice.hpp source/vr/xrdevice.cpp
    source/vr/ovr_runtime.hpp source/vr/ovr_runtime.cpp
    source/vr/ovr_overlay.hpp source/vr/ovr_overlay.cpp
    source/vr/ovr_window_overlay.cpp source/vr/ovr_window_overlay.hpp
    source/scene/root_node.hpp source/scene/root_node.cpp
    source/io/stdio_handler.hpp source/io/stdio_handler.cpp
    source/ovrpenguin.hpp source/ovrpenguin.cpp
    source/diagnostics/logger.hpp source/diagnostics/logger.cpp
    source/types/event.hpp
    source/types/mutex.hpp
    source/graphics/gl_context.cpp source/graphics/gl_context.hpp
    source/media/screen_capturer.cpp source/media/screen_capturer.hpp
    source/system/filesystem.cpp source/system/filesystem.hpp
    
    third-party/glad/src/gl.c
    )

find_package(glfw3 REQUIRED)

pkg_check_modules(PIPEWIRE IMPORTED_TARGET REQUIRED libpipewire-0.3)
foreach(path IN LISTS PIPEWIRE_LINK_LIBRARIES)
    message("PIPEWIRE/PIPEWIRE DEPENDENCY PATH: '" ${path} "'")
endforeach()

foreach(path IN LISTS PIPEWIRE_INCLUDE_DIRS)
    target_include_directories(${PROJECT_NAME} PRIVATE ${path})
    message("PIPEWIRE INCLUDE DIR: '" ${path} "'")
endforeach()

pkg_check_modules(PORTAL IMPORTED_TARGET REQUIRED libportal)
foreach(path IN LISTS PORTAL_LINK_LIBRARIES)
    message("PORTAL/PORTAL DEPENDENCY PATH: '" ${path} "'")
endforeach()

foreach(path IN LISTS PORTAL_INCLUDE_DIRS)
    target_include_directories(${PROJECT_NAME} PRIVATE ${path})
    message("PORTAL/PORTAL DEPENDENCY INCLUDE DIR: '" ${path} "'")
endforeach()

pkg_check_modules(OPENVR IMPORTED_TARGET REQUIRED openvr)
foreach(path IN LISTS OPENVR_LINK_LIBRARIES)
    message("OPENVR/OPENVR DEPENDENCY PATH: '" ${path} "'")
endforeach()

foreach(path IN LISTS PORTAL_INCLUDE_DIRS)
    target_include_directories(${PROJECT_NAME} PRIVATE ${path})
    message("OPENVR/OPENVR DEPENDENCY INCLUDE DIR: '" ${path} "'")
endforeach()

target_include_directories(${PROJECT_NAME} PRIVATE third-party/glad/include)

target_include_directories(${PROJECT_NAME} PRIVATE source)

target_link_libraries(${PROJECT_NAME}
        PkgConfig::PIPEWIRE
        PkgConfig::PORTAL
        PkgConfig::OPENVR
        glfw
)


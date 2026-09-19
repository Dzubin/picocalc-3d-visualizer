# PicoCalc 3D Visualizer - desktop SDL2 build (for developing/testing the
# renderer before flashing PicoCalc hardware - see picocalc/CMakeLists.txt
# for the real hardware build).

cmake_minimum_required(VERSION 3.13)

project(picocalc-3d-visualizer-windows C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_executable(3d-visualizer-windows
        main.c
        camera.c
        camera.h
        shapes.c
        shapes.h
        mesh_definitions.h
        hidden_line.c
        hidden_line.h
        renderer.c
        renderer.h
        text.c
        text.h
        vec3.h
        fixed.h
        gfx.h
        input.h
        timing.h
        constants.h
        color_palette.h
        desktop/gfx_desktop.c
        desktop/input_desktop.c
        desktop/timing_desktop.c
        )

target_include_directories(3d-visualizer-windows PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${SDL2_INCLUDE_DIRS}
        )

target_compile_options(3d-visualizer-windows PRIVATE -Wall -Werror ${SDL2_CFLAGS_OTHER})

# Link SDL2 (and the C runtime) statically so the .exe is a single
# self-contained file - no SDL2.dll needs to ship alongside it. The
# SDL2_STATIC_* variables come from `pkg-config --static`, which also lists
# the Windows system libraries static SDL2 depends on.
target_link_directories(3d-visualizer-windows PRIVATE ${SDL2_STATIC_LIBRARY_DIRS})
target_link_libraries(3d-visualizer-windows PRIVATE ${SDL2_STATIC_LIBRARIES})
target_link_options(3d-visualizer-windows PRIVATE -static ${SDL2_STATIC_LDFLAGS_OTHER})

# Copy the finished .exe to the top-level project folder (this file's folder).
add_custom_command(TARGET 3d-visualizer-windows POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:3d-visualizer-windows>
                ${CMAKE_CURRENT_LIST_DIR}/
        )

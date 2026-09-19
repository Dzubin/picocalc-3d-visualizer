# PicoCalc 3D Visualizer - real hardware build.
#
# Links the same portable core (../*.c) the desktop SDL2 build uses - only
# gfx_picocalc.c, input_picocalc.c and timing_picocalc.c are hardware-
# specific. Uses the LCD/keyboard/south-bridge drivers from
# picocalc-text-starter by Blair Leduc (see drivers/), vendored read-only.

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(picocalc-3d-visualizer C CXX ASM)

pico_sdk_init()

add_executable(picocalc-3d-visualizer
        ../main.c
        ../camera.c
        ../camera.h
        ../shapes.c
        ../shapes.h
        ../mesh_definitions.h
        ../hidden_line.c
        ../hidden_line.h
        ../renderer.c
        ../renderer.h
        ../text.c
        ../text.h
        ../vec3.h
        ../fixed.h
        ../gfx.h
        ../input.h
        ../timing.h
        ../constants.h
        ../color_palette.h
        gfx_picocalc.c
        input_picocalc.c
        timing_picocalc.c
        drivers/lcd.c
        drivers/lcd.h
        drivers/font-8x10.c
        drivers/font.h
        drivers/keyboard.c
        drivers/keyboard.h
        drivers/southbridge.c
        drivers/southbridge.h
        )

pico_set_program_name(picocalc-3d-visualizer "picocalc-3d-visualizer")
pico_set_program_version(picocalc-3d-visualizer "0.1")

pico_enable_stdio_uart(picocalc-3d-visualizer 0)
pico_enable_stdio_usb(picocalc-3d-visualizer 0)

target_compile_options(picocalc-3d-visualizer PRIVATE -Wall -Werror)

target_include_directories(picocalc-3d-visualizer PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/..
        )

target_link_libraries(picocalc-3d-visualizer
        pico_stdlib
        hardware_spi
        hardware_gpio
        hardware_i2c
        hardware_clocks
        )

pico_add_extra_outputs(picocalc-3d-visualizer)

# Copy the finished .uf2 to the top-level project folder with the chip name in
# its file name (picocalc-3d-visualizer-RP2040.uf2 / ...-RP2350.uf2), so the
# RP2040 and RP2350 builds can sit side by side.
if(PICO_PLATFORM MATCHES "^rp2040")
    set(CHIP_NAME RP2040)
elseif(PICO_PLATFORM MATCHES "^rp2350")
    set(CHIP_NAME RP2350)
else()
    set(CHIP_NAME ${PICO_PLATFORM})
endif()
add_custom_command(TARGET picocalc-3d-visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE_DIR:picocalc-3d-visualizer>/$<TARGET_FILE_BASE_NAME:picocalc-3d-visualizer>.uf2
                ${CMAKE_CURRENT_LIST_DIR}/../picocalc-3d-visualizer-${CHIP_NAME}.uf2
        )

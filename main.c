//
//  PicoCalc 3D Visualizer
//  =======================
//
//  A scene of placeable objects - cube, square-base pyramid, tetrahedron,
//  octagonal prism, house, line, sphere, and star, one of each - each with
//  its own hand-picked position, size, and colour (see shapes.c's
//  scene_shapes[] table), in 3D space with the X/Y/Z axes drawn through
//  the origin. The arrow keys orbit the viewpoint around the scene; F1/F2
//  move it closer/further away; the camera always faces the origin. A
//  solid object's face colour only ever shows while hidden-line removal is
//  on (see ENABLE_FACE_COLOR in constants.h); a star's colour always shows.
//
//  This is stage 3 of a multi-stage project (stage 1 was a star cloud
//  instead of solid shapes - dropped for being too slow to redraw every
//  frame on the PicoCalc's RP2350) - a later stage adds shading on top of
//  the flat face colour already here, if the RP2350 can keep up. See
//  CHANGELOG.md.
//
//  Author: Thomas Dzubin. The PicoCalc hardware backend is built on the
//  Raspberry Pi Pico SDK and the LCD/keyboard/south-bridge drivers from
//  "picocalc-text-starter" by Blair Leduc (picocalc/drivers/).
//
//  ------------------------------------------------------------------------
//  Controls
//  ------------------------------------------------------------------------
//    Arrow keys ....... orbit the camera around the scene
//    F1 ............... zoom in (move the camera toward the origin)
//    F2 ............... zoom out (move the camera away from the origin)
//    H ................ toggle hidden-line removal on/off (see hidden_line.h)
//                        - in case it's too slow on real hardware; also
//                        turns face colour on/off, since colour only ever
//                        draws alongside hidden-line removal
//    ESC .............. quit (desktop build only)
//    ~  (tilde) ....... reboot into BOOTSEL mode (PicoCalc build only)
//
//  Source layout:
//    constants.h ......... every tunable value and colour
//    vec3.h .............. portable 3D vector math
//    camera.h / .c ....... the orbit camera
//    mesh_definitions.h ... raw solid-object topology (cube, pyramid,
//                          tetrahedron, octagonal prism, house, line,
//                          sphere) - not meant to be edited, see its own
//                          warning
//    shapes.h / .c ....... every placeable object (solid shapes AND
//                          stars) - a plain, hand-edited scene table at
//                          the top of shapes.c is the one place to
//                          add/move/remove/resize/recolour one
//    hidden_line.h / .c ... hidden-line removal (toggle: H) and, while it's
//                          on, flat per-face colour fill
//    renderer.h / .c ..... projects and draws the axes + every object
//    gfx.h, input.h, timing.h ... the only platform seams; implemented by
//      desktop/ (SDL2) and picocalc/ (real hardware)
//

#include "constants.h"
#include "gfx.h"
#include "input.h"
#include "timing.h"
#include "camera.h"
#include "shapes.h"
#include "hidden_line.h"
#include "renderer.h"

static void apply_input(camera_t *cam, const input_state_t *in, float dt_seconds);

// Author: Thomas Dzubin
int main(void)
{
    camera_t cam;
    float dt_seconds = FRAME_MS / 1000.0f;

    gfx_init();
    input_init();
    camera_init(&cam, 30.0f, 20.0f);

    for (;;) {
        input_state_t in;
        unsigned long frame_start_us = platform_now_us();
        unsigned long elapsed_us, target_us;

        input_poll(&in);
        if (in.quit) break;
        if (in.toggle_hidden_line) hidden_line_toggle();

        apply_input(&cam, &in, dt_seconds);
        renderer_draw_frame(&cam);

        /* Sleep only what's left of the FRAME_MS budget, not a flat
           FRAME_MS on top of however long input+render just took. On the
           PicoCalc this budget is frequently already gone (render+blit
           alone can exceed FRAME_MS), so those frames sleep 0 and just run
           back to back at whatever rate the work itself allows. */
        elapsed_us = platform_now_us() - frame_start_us;
        target_us = (unsigned long)FRAME_MS * 1000u;
        if (elapsed_us < target_us) {
            platform_sleep_ms((int)((target_us - elapsed_us) / 1000u));
        }
    }

    return 0;
}

// Author: Thomas Dzubin
static void apply_input(camera_t *cam, const input_state_t *in, float dt_seconds)
{
    float step = ORBIT_TURN_SPEED_DEG_PER_SEC * dt_seconds;
    float zoom_step = ORBIT_ZOOM_SPEED_UNITS_PER_SEC * dt_seconds;
    float d_azimuth = 0.0f, d_elevation = 0.0f;

    if (in->left)  d_azimuth -= step;
    if (in->right) d_azimuth += step;
    if (in->up)    d_elevation += step;
    if (in->down)  d_elevation -= step;

    camera_orbit(cam, d_azimuth, d_elevation);

    if (in->zoom_in)  camera_zoom(cam, -zoom_step);
    if (in->zoom_out) camera_zoom(cam, zoom_step);
}

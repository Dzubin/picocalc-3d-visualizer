//
//  input.h - the only input source the portable core touches. Each platform
//  backend (desktop/input_desktop.c, picocalc/input_picocalc.c) fills an
//  input_state_t from whatever its keyboard actually looks like.
//
//  '~' (BOOTSEL reboot) never appears here - it is handled directly inside
//  the PicoCalc backend, the same way every other PicoCalc project in this
//  workspace does it.
//

#ifndef INPUT_H
#define INPUT_H

typedef struct {
    int left, right;         // orbit azimuth
    int up, down;             // orbit elevation
    int zoom_in, zoom_out;    // F1/F2: move the camera toward/away from the origin (see camera_zoom())
    int toggle_hidden_line;   // true for exactly one input_poll() call per H key press (see hidden_line.h)
    int quit;
} input_state_t;

void input_init(void);
void input_poll(input_state_t *state);

#endif

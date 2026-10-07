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
    int left, right;          // arrow keys: move over the sphere round the vertical axis (see camera_orbit())
    int up, down;             // arrow keys: move up / down over the sphere
    int zoom_in, zoom_out;    // F1 / F2: shrink / grow the sphere (see camera_zoom())
    int pitch_up, pitch_down; // p / P (SHIFT + p): tilt the view up / down (see camera_turn())
    int yaw_right, yaw_left;  // y / Y: turn the view right / left
    int roll_right, roll_left; // r / R: bank the view right / left
    int reset_camera;         // z or Z: back to the starting view (true while the key is down)
    int toggle_hidden_line;   // true for exactly one input_poll() call per H key press (see hidden_line.h)
    int quit;
} input_state_t;

void input_init(void);
void input_poll(input_state_t *state);

#endif

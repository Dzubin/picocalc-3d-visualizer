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
    int toggle_hidden_line;   // true for exactly one input_poll() call per L key press (see hidden_line.h)
    int show_help;            // true for exactly one input_poll() call per H key press (see help.h)
    int show_title;           // true for exactly one input_poll() call per Q or ESC press: back to the title screen
    int quit;                 // the desktop window was closed
} input_state_t;

void input_init(void);
void input_poll(input_state_t *state);

// What input_wait_for_key() saw.
typedef enum {
    INPUT_KEY_ANY,      // some other key
    INPUT_KEY_LEAVE,    // Q or ESC (either case)
    INPUT_KEY_YES,      // Y (either case)
    INPUT_KEY_CLOSED    // the desktop window was closed
} input_key_t;

// Blocks until a key is pressed (any key), for the title, help and confirm
// screens, and says which kind it was. A key that is still held down from
// before the call (the H that opened the help) does not count: on the
// PicoCalc, whose keyboard repeats a held key, it waits until the repeating
// has stopped first. '~' still reboots into BOOTSEL on the PicoCalc.
input_key_t input_wait_for_key(void);

// Leaves the program for good. On the PicoCalc that is back to the UF2 Loader
// menu (it asks the loader for its menu through the watchdog scratch registers;
// with no loader installed the program just restarts); on the desktop the
// program ends. Does not return.
void input_leave_program(void);
#endif

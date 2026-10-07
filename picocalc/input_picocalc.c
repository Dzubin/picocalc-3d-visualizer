#include "../input.h"
#include "../constants.h"
#include "drivers/keyboard.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"   /* watchdog_hw, watchdog_reboot() - exit to the UF2 Loader */

/* Referenced by drivers/keyboard.c (set when the Brk key is pressed). Not
   used here, but the driver needs the symbol to link. */
volatile bool user_interrupt = false;

/* Leaves the program for the PicoCalc UF2 Loader's menu: asks the loader for
   its menu (see LOADER_COMMAND_MAGIC in constants.h) and reboots with the
   watchdog. If the program was flashed straight to the chip with no loader,
   nothing reads the request and it simply restarts. Never returns. */
static void exit_to_loader(void)
{
    watchdog_hw->scratch[LOADER_SCRATCH_MODE] = LOADER_BOOT_MODE_SD;
    watchdog_hw->scratch[LOADER_SCRATCH_ARGUMENT] = 0;
    watchdog_hw->scratch[LOADER_SCRATCH_MAGIC] = LOADER_COMMAND_MAGIC;
    watchdog_reboot(0, 0, LOADER_REBOOT_DELAY_MS);

    for (;;)
        tight_loop_contents();      /* the reboot comes in a few milliseconds */
}

/* The keyboard driver auto-repeats a held key into its queue roughly every
   100ms (KEY_STATE_HOLD - see drivers/keyboard.c) and never reports a
   release for ordinary keys, so "currently held" is reconstructed here: a
   control (an arrow, F1/F2, or a p, y or r key, capital or not) counts as
   held for PICOCALC_KEY_HOLD_TIMEOUT_FRAMES frames after its last event,
   which comfortably bridges the ~100ms repeat gaps at our own FRAME_MS and
   gives smooth flying and turning instead of visible steps. A capital
   letter is the same key with SHIFT, and is the opposite direction.

   H uses the same counter for the opposite purpose: it should toggle
   hidden-line removal once per physical press, not repeatedly while held,
   so the toggle only fires when the counter has reached 0 (i.e. no H event
   seen recently) and is otherwise just re-armed like the others. */
enum {
    HELD_LEFT, HELD_RIGHT, HELD_UP, HELD_DOWN,
    HELD_RISE, HELD_DESCEND,
    HELD_PITCH_UP, HELD_PITCH_DOWN,
    HELD_YAW_RIGHT, HELD_YAW_LEFT,
    HELD_ROLL_RIGHT, HELD_ROLL_LEFT,
    HELD_RESET,
    HELD_H,
    HELD_COUNT
};

static int ticks[HELD_COUNT];

void input_init(void)
{
    int i;

    keyboard_init();
    keyboard_set_background_poll(true);
    for (i = 0; i < HELD_COUNT; i++) ticks[i] = 0;
}

// Author: Thomas Dzubin
void input_poll(input_state_t *state)
{
    int toggle_hidden_line = 0;
    int i;

    while (keyboard_key_available()) {
        char key = keyboard_get_key();

        switch (key) {
        case '~':
            rom_reset_usb_boot(0, 0);
            break;
        case KEY_ESC:
        case 'Q':
        case 'q':
            exit_to_loader();       /* does not return */
            break;
        case KEY_LEFT:  ticks[HELD_LEFT] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_RIGHT: ticks[HELD_RIGHT] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_UP:    ticks[HELD_UP] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_DOWN:  ticks[HELD_DOWN] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_F1:    ticks[HELD_RISE] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_F2:    ticks[HELD_DESCEND] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'p': ticks[HELD_PITCH_UP] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'P': ticks[HELD_PITCH_DOWN] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'y': ticks[HELD_YAW_RIGHT] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'Y': ticks[HELD_YAW_LEFT] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'r': ticks[HELD_ROLL_RIGHT] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'R': ticks[HELD_ROLL_LEFT] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'z':
        case 'Z': ticks[HELD_RESET] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'H':
        case 'h':
            if (ticks[HELD_H] == 0) toggle_hidden_line = 1;
            ticks[HELD_H] = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES;
            break;
        default: break;
        }
    }

    state->left = ticks[HELD_LEFT] > 0;
    state->right = ticks[HELD_RIGHT] > 0;
    state->up = ticks[HELD_UP] > 0;
    state->down = ticks[HELD_DOWN] > 0;
    state->rise = ticks[HELD_RISE] > 0;
    state->descend = ticks[HELD_DESCEND] > 0;
    state->pitch_up = ticks[HELD_PITCH_UP] > 0;
    state->pitch_down = ticks[HELD_PITCH_DOWN] > 0;
    state->yaw_right = ticks[HELD_YAW_RIGHT] > 0;
    state->yaw_left = ticks[HELD_YAW_LEFT] > 0;
    state->roll_right = ticks[HELD_ROLL_RIGHT] > 0;
    state->roll_left = ticks[HELD_ROLL_LEFT] > 0;
    state->reset_camera = ticks[HELD_RESET] > 0;
    state->toggle_hidden_line = toggle_hidden_line;
    state->quit = 0;   /* ESC / Q reboot into the UF2 Loader above, '~' into BOOTSEL */

    for (i = 0; i < HELD_COUNT; i++) {
        if (ticks[i] > 0) ticks[i]--;
    }
}

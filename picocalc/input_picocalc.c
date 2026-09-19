#include "../input.h"
#include "../constants.h"
#include "drivers/keyboard.h"
#include "pico/bootrom.h"

/* Referenced by drivers/keyboard.c (set when the Brk key is pressed). Not
   used here, but the driver needs the symbol to link. */
volatile bool user_interrupt = false;

/* The keyboard driver auto-repeats a held key into its queue roughly every
   100ms (KEY_STATE_HOLD - see drivers/keyboard.c) and never reports a
   release for ordinary keys, so "currently held" is reconstructed here: a
   direction (or F1/F2 zoom) counts as held for
   PICOCALC_KEY_HOLD_TIMEOUT_FRAMES frames after its last event, which
   comfortably bridges the ~100ms repeat gaps at our own FRAME_MS and gives
   smooth orbiting/zooming instead of visible steps.

   h_hold_ticks uses the same counter for the opposite purpose: H should
   toggle hidden-line removal once per physical press, not repeatedly while
   held, so the toggle only fires when the counter has reached 0 (i.e. no H
   event seen recently) and is otherwise just re-armed like the direction
   counters. */
static int left_ticks, right_ticks, up_ticks, down_ticks;
static int zoom_in_ticks, zoom_out_ticks, h_hold_ticks;

void input_init(void)
{
    keyboard_init();
    keyboard_set_background_poll(true);
    left_ticks = right_ticks = up_ticks = down_ticks = 0;
    zoom_in_ticks = zoom_out_ticks = h_hold_ticks = 0;
}

// Author: Thomas Dzubin
void input_poll(input_state_t *state)
{
    int toggle_hidden_line = 0;

    while (keyboard_key_available()) {
        char key = keyboard_get_key();

        switch (key) {
        case '~':
            rom_reset_usb_boot(0, 0);
            break;
        case KEY_LEFT:  left_ticks = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_RIGHT: right_ticks = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_UP:    up_ticks = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_DOWN:  down_ticks = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_F1:    zoom_in_ticks = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case KEY_F2:    zoom_out_ticks = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES; break;
        case 'H':
        case 'h':
            if (h_hold_ticks == 0) toggle_hidden_line = 1;
            h_hold_ticks = PICOCALC_KEY_HOLD_TIMEOUT_FRAMES;
            break;
        default: break;
        }
    }

    state->left = left_ticks > 0;
    state->right = right_ticks > 0;
    state->up = up_ticks > 0;
    state->down = down_ticks > 0;
    state->zoom_in = zoom_in_ticks > 0;
    state->zoom_out = zoom_out_ticks > 0;
    state->toggle_hidden_line = toggle_hidden_line;
    state->quit = 0;   /* nothing quits on PicoCalc - '~' reboots instead */

    if (left_ticks > 0) left_ticks--;
    if (right_ticks > 0) right_ticks--;
    if (up_ticks > 0) up_ticks--;
    if (down_ticks > 0) down_ticks--;
    if (zoom_in_ticks > 0) zoom_in_ticks--;
    if (zoom_out_ticks > 0) zoom_out_ticks--;
    if (h_hold_ticks > 0) h_hold_ticks--;
}

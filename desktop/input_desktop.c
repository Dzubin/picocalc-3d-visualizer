#include "../input.h"
#include <SDL.h>
#include <stdlib.h>

void input_init(void)
{
    /* SDL_Init(SDL_INIT_VIDEO) already ran in gfx_init(), which main()
       calls first - SDL's event/keyboard state needs that same init. */
}

// Author: Thomas Dzubin
void input_poll(input_state_t *state)
{
    SDL_Event event;
    const Uint8 *keys;
    int shifted;

    state->left = state->right = state->up = state->down = 0;
    state->zoom_in = state->zoom_out = 0;
    state->pitch_up = state->pitch_down = 0;
    state->yaw_right = state->yaw_left = 0;
    state->roll_right = state->roll_left = 0;
    state->reset_camera = 0;
    state->toggle_hidden_line = state->show_help = state->show_title = state->quit = 0;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) state->quit = 1;
        /* !event.key.repeat: fire once on the initial press, not on every
           OS auto-repeat KEYDOWN while L or H is held - a toggle should flip
           once per physical press. */
        if (event.type == SDL_KEYDOWN && !event.key.repeat) {
            if (event.key.keysym.scancode == SDL_SCANCODE_L) state->toggle_hidden_line = 1;
            if (event.key.keysym.scancode == SDL_SCANCODE_H) state->show_help = 1;
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ||
                event.key.keysym.scancode == SDL_SCANCODE_Q) state->show_title = 1;
        }
    }

    keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_LEFT])  state->left = 1;
    if (keys[SDL_SCANCODE_RIGHT]) state->right = 1;
    if (keys[SDL_SCANCODE_UP])    state->up = 1;
    if (keys[SDL_SCANCODE_DOWN])  state->down = 1;
    if (keys[SDL_SCANCODE_F1])    state->zoom_in = 1;
    if (keys[SDL_SCANCODE_F2])    state->zoom_out = 1;

    /* p, y and r turn one way, and the same key with SHIFT the other, like
       the capital letters the PicoCalc keyboard sends. */
    shifted = (SDL_GetModState() & KMOD_SHIFT) != 0;
    if (keys[SDL_SCANCODE_P]) { if (shifted) state->pitch_down = 1; else state->pitch_up = 1; }
    if (keys[SDL_SCANCODE_Y]) { if (shifted) state->yaw_left = 1;   else state->yaw_right = 1; }
    if (keys[SDL_SCANCODE_R]) { if (shifted) state->roll_left = 1;  else state->roll_right = 1; }
    if (keys[SDL_SCANCODE_Z]) state->reset_camera = 1;
}

// Author: Thomas Dzubin
input_key_t input_wait_for_key(void)
{
    SDL_Event event;

    /* A key that was already down when the help screen opened has had its one
       KEYDOWN (this ignores the OS auto-repeat ones), so the first KEYDOWN
       from here on is a new press. */
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) return INPUT_KEY_CLOSED;
        if (event.type == SDL_KEYDOWN && !event.key.repeat) {
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
            case SDL_SCANCODE_Q:
                return INPUT_KEY_LEAVE;
            case SDL_SCANCODE_Y:
                return INPUT_KEY_YES;
            default:
                return INPUT_KEY_ANY;
            }
        }
    }
    return INPUT_KEY_CLOSED;
}

void input_leave_program(void)
{
    SDL_Quit();
    exit(0);
}

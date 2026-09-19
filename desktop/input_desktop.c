#include "../input.h"
#include <SDL.h>

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

    state->left = state->right = state->up = state->down = 0;
    state->zoom_in = state->zoom_out = 0;
    state->toggle_hidden_line = state->quit = 0;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) state->quit = 1;
        /* !event.key.repeat: fire once on the initial press, not on every
           OS auto-repeat KEYDOWN while H is held - a toggle should flip
           once per physical press. */
        if (event.type == SDL_KEYDOWN && !event.key.repeat &&
            event.key.keysym.scancode == SDL_SCANCODE_H) {
            state->toggle_hidden_line = 1;
        }
    }

    keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_LEFT])  state->left = 1;
    if (keys[SDL_SCANCODE_RIGHT]) state->right = 1;
    if (keys[SDL_SCANCODE_UP])    state->up = 1;
    if (keys[SDL_SCANCODE_DOWN])  state->down = 1;
    if (keys[SDL_SCANCODE_F1])    state->zoom_in = 1;
    if (keys[SDL_SCANCODE_F2])    state->zoom_out = 1;
    if (keys[SDL_SCANCODE_ESCAPE]) state->quit = 1;
}

#include "../gfx.h"
#include "../constants.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ENABLE_INDEXED_COLOR
#include "../color_palette.h"
#endif

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

#if ENABLE_INDEXED_COLOR
static unsigned char framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
#else
static unsigned short framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
#endif

// Author: Thomas Dzubin
void gfx_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("PicoCalc 3D Visualizer",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               SCREEN_WIDTH * DESKTOP_WINDOW_SCALE,
                               SCREEN_HEIGHT * DESKTOP_WINDOW_SCALE,
                               0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 SCREEN_WIDTH, SCREEN_HEIGHT);
}

#if ENABLE_INDEXED_COLOR

void gfx_clear(unsigned short color)
{
    memset(framebuffer, color_palette_index(color), sizeof framebuffer);
}

void gfx_set_pixel(int x, int y, unsigned short color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    framebuffer[y][x] = color_palette_index(color);
}

/* Desktop has plenty of RAM, so unlike gfx_picocalc.c this just expands the
   whole indexed frame into a full-size RGB565 scratch buffer in one go
   rather than a chunk at a time - the point of doing this on PicoCalc is
   fitting an RP2040's RAM budget, which doesn't apply here. */
// Author: Thomas Dzubin
void gfx_present(void)
{
    static unsigned short expanded[SCREEN_HEIGHT][SCREEN_WIDTH];
    int x, y;

    for (y = 0; y < SCREEN_HEIGHT; y++) {
        for (x = 0; x < SCREEN_WIDTH; x++) {
            expanded[y][x] = color_palette[framebuffer[y][x]];
        }
    }

    SDL_UpdateTexture(texture, NULL, expanded, SCREEN_WIDTH * (int)sizeof(unsigned short));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

#else /* !ENABLE_INDEXED_COLOR */

void gfx_clear(unsigned short color)
{
    int x, y;
    for (y = 0; y < SCREEN_HEIGHT; y++) {
        for (x = 0; x < SCREEN_WIDTH; x++) {
            framebuffer[y][x] = color;
        }
    }
}

void gfx_set_pixel(int x, int y, unsigned short color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    framebuffer[y][x] = color;
}

void gfx_present(void)
{
    SDL_UpdateTexture(texture, NULL, framebuffer, SCREEN_WIDTH * (int)sizeof(unsigned short));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

#endif /* ENABLE_INDEXED_COLOR */

#include "../gfx.h"
#include "../constants.h"
#include "drivers/lcd.h"
#include <string.h>

#if ENABLE_INDEXED_COLOR
#include "../color_palette.h"
#endif

#if ENABLE_INDEXED_COLOR
static unsigned char framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
#else
static uint16_t framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
#endif

void gfx_init(void)
{
    lcd_init();               /* also clears the screen and turns the display on */
    lcd_enable_cursor(false); /* we never draw the text cursor */
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

// How many rows are expanded to real RGB565 into the scratch buffer below
// and sent per lcd_blit() call. The whole point of ENABLE_INDEXED_COLOR is
// RAM, not speed - the LCD only understands RGB565, so the same ~200KB of
// pixel data still goes out over SPI either way - so this never needs a
// second full-size buffer, just a small one reused a chunk at a time. Too
// few rows per chunk means more lcd_blit() calls, each with its own
// address-window setup overhead; too many means a bigger scratch buffer
// for no real benefit - 40 rows (320 * 40 * 2 = 25KB) is a middle ground,
// not a carefully tuned number.
#define GFX_BLIT_CHUNK_ROWS 40

// Author: Thomas Dzubin
void gfx_present(void)
{
    static unsigned short chunk[GFX_BLIT_CHUNK_ROWS][SCREEN_WIDTH];
    int y0;

    for (y0 = 0; y0 < SCREEN_HEIGHT; y0 += GFX_BLIT_CHUNK_ROWS) {
        int rows = GFX_BLIT_CHUNK_ROWS;
        int row, x;

        if (y0 + rows > SCREEN_HEIGHT) rows = SCREEN_HEIGHT - y0;

        for (row = 0; row < rows; row++) {
            for (x = 0; x < SCREEN_WIDTH; x++) {
                chunk[row][x] = color_palette[framebuffer[y0 + row][x]];
            }
        }
        lcd_blit(&chunk[0][0], 0, y0, SCREEN_WIDTH, rows);
    }
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
    lcd_blit(&framebuffer[0][0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

#endif /* ENABLE_INDEXED_COLOR */

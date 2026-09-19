#include "text.h"
#include "gfx.h"

#define GLYPH_W 3
#define GLYPH_H 5
#define GLYPH_PIXEL_SCALE 3
#define GLYPH_SPACING_PX 2

typedef struct {
    char c;
    unsigned char rows[GLYPH_H];   // each row: bits 2,1,0 = columns left-to-right
} glyph_t;

static const glyph_t glyphs[] = {
    {'0', {0x7, 0x5, 0x5, 0x5, 0x7}},
    {'1', {0x2, 0x6, 0x2, 0x2, 0x7}},
    {'2', {0x7, 0x1, 0x7, 0x4, 0x7}},
    {'3', {0x7, 0x1, 0x7, 0x1, 0x7}},
    {'4', {0x5, 0x5, 0x7, 0x1, 0x1}},
    {'5', {0x7, 0x4, 0x7, 0x1, 0x7}},
    {'6', {0x7, 0x4, 0x7, 0x5, 0x7}},
    {'7', {0x7, 0x1, 0x2, 0x2, 0x2}},
    {'8', {0x7, 0x5, 0x7, 0x5, 0x7}},
    {'9', {0x7, 0x5, 0x7, 0x1, 0x7}},
    {'-', {0x0, 0x0, 0x7, 0x0, 0x0}},
    {'X', {0x5, 0x5, 0x2, 0x5, 0x5}},
    {'Y', {0x5, 0x5, 0x2, 0x2, 0x2}},
    {'Z', {0x7, 0x1, 0x2, 0x4, 0x7}},
    {':', {0x0, 0x2, 0x0, 0x2, 0x0}},
    {'H', {0x5, 0x5, 0x7, 0x5, 0x5}},
    {'L', {0x4, 0x4, 0x4, 0x4, 0x7}},
    {'R', {0x7, 0x5, 0x6, 0x5, 0x5}},
    {'B', {0x6, 0x5, 0x6, 0x5, 0x6}},
    {'E', {0x7, 0x4, 0x7, 0x4, 0x7}},
    {'N', {0x5, 0x7, 0x7, 0x5, 0x5}},
    {'D', {0x6, 0x5, 0x5, 0x5, 0x6}},
    {'I', {0x7, 0x2, 0x2, 0x2, 0x7}},
    {'T', {0x7, 0x2, 0x2, 0x2, 0x2}},
    {'S', {0x7, 0x4, 0x7, 0x1, 0x7}},
    {'K', {0x5, 0x6, 0x4, 0x6, 0x5}},
    {'=', {0x0, 0x7, 0x0, 0x7, 0x0}},
};
#define GLYPH_COUNT ((int)(sizeof(glyphs) / sizeof(glyphs[0])))

static const unsigned char *find_glyph(char c)
{
    int i;
    for (i = 0; i < GLYPH_COUNT; i++) {
        if (glyphs[i].c == c) return glyphs[i].rows;
    }
    return 0;   // space or anything unsupported: blank advance
}

static int char_advance(void)
{
    return GLYPH_W * GLYPH_PIXEL_SCALE + GLYPH_SPACING_PX;
}

int text_width(const char *s)
{
    int len = 0;
    while (s[len]) len++;
    return len > 0 ? len * char_advance() - GLYPH_SPACING_PX : 0;
}

// Author: Thomas Dzubin
void text_draw(int x, int y, const char *s, unsigned short color)
{
    for (; *s; s++) {
        const unsigned char *rows = find_glyph(*s);
        if (rows) {
            int row, col;
            for (row = 0; row < GLYPH_H; row++) {
                for (col = 0; col < GLYPH_W; col++) {
                    if (rows[row] & (1 << (GLYPH_W - 1 - col))) {
                        int px = x + col * GLYPH_PIXEL_SCALE;
                        int py = y + row * GLYPH_PIXEL_SCALE;
                        int dx, dy;
                        for (dy = 0; dy < GLYPH_PIXEL_SCALE; dy++) {
                            for (dx = 0; dx < GLYPH_PIXEL_SCALE; dx++) {
                                gfx_set_pixel(px + dx, py + dy, color);
                            }
                        }
                    }
                }
            }
        }
        x += char_advance();
    }
}

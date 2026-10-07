//
//  text.h - a tiny built-in pixel font, drawn through gfx.h so it works
//  identically on both backends with no platform text API of any kind.
//  Two fonts. The small hand-made one (text_draw()) only covers the characters
//  the status display needs: digits, '-', ':',
//  '=', and the uppercase letters X, Y, Z, H, L, R, B, E, N, D, I, T, S, K, P.
//  No lowercase, no space glyph (an unrecognized character - including a
//  literal space - just advances the cursor with nothing drawn, see
//  find_glyph() in text.c). No platform dependencies.
//

#ifndef TEXT_H
#define TEXT_H

// Width in pixels a string would occupy if drawn with text_draw().
int text_width(const char *s);

// Draws a string with its top-left corner at (x, y).
void text_draw(int x, int y, const char *s, unsigned short color);

// The PicoCalc's own 8x10 font (the glyph table in picocalc/drivers/font-8x10.c,
// full ASCII with lower case), for the full-screen text screens. Each
// character is a cell 8 pixels wide and 10 high; scale draws every pixel of it
// as a scale x scale block, so scale 2 gives 16 x 20.
void text_draw_8x10(int x, int y, const char *s, unsigned short color, int scale);

// Width in pixels of a string drawn with text_draw_8x10().
int text_width_8x10(const char *s, int scale);

// The height of one line of that font, before scaling.
#define TEXT_8X10_HEIGHT 10
#define TEXT_8X10_WIDTH 8

#endif

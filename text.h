//
//  text.h - a tiny built-in pixel font, drawn through gfx.h so it works
//  identically on both backends with no platform text API of any kind.
//  Only covers the characters the status display needs: digits, '-', ':',
//  '=', and the uppercase letters X, Y, Z, H, L, R, B, E, N, D, I, T, S, K.
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

#endif

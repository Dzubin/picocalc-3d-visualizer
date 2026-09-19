//
//  color_palette.h - shared by both gfx_desktop.c and gfx_picocalc.c for
//  the experimental indexed colour framebuffer (see ENABLE_INDEXED_COLOR
//  in constants.h). Not part of the portable core - renderer.c,
//  hidden_line.c, etc. never see a palette index, only ever a real RGB565
//  value passed to gfx_set_pixel(); only a backend's own internal storage
//  format changes.
//
//  This list must be kept in sync BY HAND with constants.h: it mirrors
//  FACE_COLOR_PALETTE's 10 entries (same order, same indices 0-9) plus the
//  3 axis colours, since those 13 are the only RGB565 values this project
//  ever actually draws. A colour added to constants.h without a matching
//  entry here silently falls back to color_palette_index()'s default
//  (index 0, black) instead of failing loudly - deliberately simple over
//  automatic, since deriving this table from FACE_COLOR_PALETTE at compile
//  time would mean indexing a static const array inside another static
//  initializer, which isn't reliably a constant expression in portable C.
//

#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include "constants.h"

static const unsigned short color_palette[] = {
    COLOR_BLACK,         // 0
    COLOR_FACE_WHITE,    // 1
    COLOR_FACE_RED,      // 2
    COLOR_FACE_GREEN,    // 3
    COLOR_FACE_BLUE,     // 4
    COLOR_FACE_AMBER,    // 5
    COLOR_FACE_PURPLE,   // 6
    COLOR_FACE_TEAL,     // 7
    COLOR_FACE_ORANGE,   // 8
    COLOR_FACE_MAGENTA,  // 9
    COLOR_AXIS_X,        // 10
    COLOR_AXIS_Y,        // 11
    COLOR_AXIS_Z,        // 12
};
#define COLOR_PALETTE_SIZE ((int)(sizeof(color_palette) / sizeof(color_palette[0])))

// Finds color's index in color_palette, or 0 (black) if it isn't one of
// the known colours - a linear scan over ~13 entries, called once per
// gfx_set_pixel()/gfx_clear(), the same cost category as everything else
// in that per-pixel path.
static inline unsigned char color_palette_index(unsigned short color)
{
    int i;
    for (i = 0; i < COLOR_PALETTE_SIZE; i++) {
        if (color_palette[i] == color) return (unsigned char)i;
    }
    return 0;
}

#endif

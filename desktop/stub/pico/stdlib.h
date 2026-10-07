//
//  A stand-in for the Pico SDK's pico/stdlib.h, for the desktop build only.
//  picocalc/drivers/font.h includes it just to get the fixed-width integer
//  types, and the desktop build compiles picocalc/drivers/font-8x10.c for its
//  glyph table (see text.c). Nothing else here uses it.
//

#ifndef DESKTOP_STUB_PICO_STDLIB_H
#define DESKTOP_STUB_PICO_STDLIB_H

#include <stdint.h>
#include <stdbool.h>

#endif

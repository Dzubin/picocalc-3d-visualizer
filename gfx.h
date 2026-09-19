//
//  gfx.h - the only drawing surface the portable core touches. Each
//  platform backend (desktop/gfx_desktop.c, picocalc/gfx_picocalc.c)
//  implements this against its own screen.
//

#ifndef GFX_H
#define GFX_H

void gfx_init(void);
void gfx_clear(unsigned short color);
void gfx_set_pixel(int x, int y, unsigned short color);
void gfx_present(void);

#endif

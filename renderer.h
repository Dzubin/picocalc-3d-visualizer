//
//  renderer.h - draws one frame (the X/Y/Z axes plus the star cloud) as
//  seen from a camera. No platform dependencies - draws only through gfx.h.
//

#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"

void renderer_draw_frame(const camera_t *cam);

#endif

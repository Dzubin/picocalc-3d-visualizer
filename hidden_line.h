//
//  hidden_line.h - hidden-line removal: a per-pixel occluder depth buffer,
//  filled by rasterizing each shape's front-facing faces, then queried
//  while drawing lines so segments (or parts of segments) behind an opaque
//  face - the same shape's own far side, or a different shape entirely -
//  aren't drawn. Runtime-togglable (see hidden_line_toggle()) in case it's
//  too slow on real hardware.
//
//  hidden_line_pixel_visible()'s depth parameter type depends on
//  USE_FIXED_POINT_MATH (see constants.h): a view-space float z normally,
//  or a quantized-1/z int when the fixed-point hot path is enabled - see
//  hidden_line.c's header comment for why the fixed-point path is built
//  around 1/z instead of z. Either way the only caller is renderer.c's
//  draw_line(), which is compiled to match (its own USE_FIXED_POINT_MATH
//  block computes whichever this build expects), so nothing else needs to
//  know which mode is active.
//

#ifndef HIDDEN_LINE_H
#define HIDDEN_LINE_H

#include "constants.h"
#include "camera.h"
#include "vec3.h"

void hidden_line_set_enabled(int enabled);
int hidden_line_enabled(void);
void hidden_line_toggle(void);

// Clears the occluder depth buffer. Call once per frame, before
// rasterizing occluders, only when hidden_line_enabled().
void hidden_line_begin_frame(void);

// Rasterizes one already-known-front-facing, convex, planar world-space
// polygon (a shape face - a triangle or a quad, any vertex count works via
// fan triangulation) into the occluder depth buffer. A no-op when hidden-
// line removal is disabled.
//
// When fill is true, also paints fill_color into the colour framebuffer for
// every pixel of this polygon that wins its depth test (i.e. every pixel
// where this face turns out to be the closest thing rasterized there so
// far this frame) - the same test that already decides occlusion, so flat
// per-face colour comes out correctly front-to-back ordered with no
// separate pass or sort. See ENABLE_FACE_COLOR in constants.h.
void hidden_line_rasterize_occluder_polygon(const camera_t *cam, const vec3_t *vertices, int vertex_count,
                                             int fill, unsigned short fill_color);

#if USE_FIXED_POINT_MATH
// True if screen pixel (x, y) at quantized depth qinvz (0..HIDDEN_LINE_DEPTH_MAX,
// larger = closer - see hidden_line.c) is not hidden behind anything already
// rasterized there this frame. Always true when hidden-line removal is
// disabled.
int hidden_line_pixel_visible(int x, int y, int qinvz);

// Maps a view-space depth to the same 0..HIDDEN_LINE_DEPTH_MAX "larger =
// closer" scale hidden_line_pixel_visible() and the occluder rasterizer
// both use - the one place that mapping is defined, so renderer.c's
// draw_world_segment() can use it too instead of duplicating the formula.
int hidden_line_quantize_view_z(float z);
#else
// True if a point at screen pixel (x, y) and view-space depth z is not
// hidden behind anything already rasterized there this frame. Always true
// when hidden-line removal is disabled.
int hidden_line_pixel_visible(int x, int y, float z);
#endif

#endif

//
//  constants.h - every tunable value and colour for PicoCalc 3D Visualizer.
//
//  Included by the portable core and both platform backends. Change
//  appearance/behaviour here, not in the .c files.
//

#pragma once

// --- Program identity --------------------------------------------------
#define VERSION "V1.0RC1"

// --- Experimental fixed-point hot path --------------------------------------
// Set to 1 to route the per-frame/per-pixel rendering math (perspective
// projection, and hidden-line removal's depth interpolation and triangle
// rasterizer - see the USE_FIXED_POINT_MATH blocks in renderer.c and
// hidden_line.c) through Q16.16 fixed-point integers (fixed.h) instead of
// float, to compare frame rate on real hardware. World-space math (the
// camera transform in camera.c, shape placement in shapes.c) stays float
// either way - those run once or a few hundred times a frame, not once per
// pixel, so they were never the target. Flip back to 0 and rebuild to
// return to the original float path exactly - nothing else needs to change.
#define USE_FIXED_POINT_MATH 1

// --- Display -------------------------------------------------------------
// The PicoCalc's real LCD is 320x320; the desktop SDL2 backend renders the
// same 320x320 logical framebuffer, scaled up for visibility (see
// DESKTOP_WINDOW_SCALE), so both backends draw identical coordinates.
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 320

#define DESKTOP_WINDOW_SCALE 2

// --- Colours (RGB565 - matches both the LCD controller's native format and
// SDL_PIXELFORMAT_RGB565, so the framebuffer is byte-identical on both
// backends) --------------------------------------------------------------
#define RGB565(r,g,b) ((unsigned short)(((((r) & 0xFF) >> 3) << 11) | \
                                          ((((g) & 0xFF) >> 2) << 5)  | \
                                          (((b) & 0xFF) >> 3)))

#define COLOR_BLACK       RGB565(0, 0, 0)
#define COLOR_SHAPE       RGB565(255, 255, 255)
#define COLOR_AXIS_X      RGB565(255, 60, 60)      // red
#define COLOR_AXIS_Y      RGB565(60, 255, 60)       // green
#define COLOR_AXIS_Z      RGB565(90, 140, 255)      // blue
#define COLOR_STATUS      RGB565(255, 255, 255)

// --- Face/star colouring (experimental) ----------------------------------
// Flat, unshaded colour, chosen per scene entry (see shapes.c's
// shape_instance_t.color / the scene_shapes[] table) as a single 0-9 index
// into FACE_COLOR_PALETTE below - see shape_color() in shapes.c, which
// resolves an index to the actual RGB565 value everything else uses.
//
// For a solid object (cube/pyramid/tetrahedron), the colour is only ever
// drawn while hidden-line removal is on: hidden_line.c's occluder
// rasterizer only fills a pixel's colour at the same moment it wins that
// pixel's depth test (the same test that already decides occlusion), so
// colouring naturally respects front-to-back ordering with no separate
// sorting pass, and naturally never happens when HLR is off (occluder
// rasterization doesn't run at all then - see renderer_draw_frame()). Set
// ENABLE_FACE_COLOR to 0 and rebuild to go back to wireframe-only for solid
// objects - shape_color() still resolves an index per entry, it's just
// never painted onto a face. Edges are always drawn in COLOR_SHAPE on top,
// regardless of this setting.
//
// For a star (OBJECT_STAR - see shapes.c), the colour is just the single
// pixel's colour, drawn regardless of ENABLE_FACE_COLOR or hidden-line
// removal - same as this project's original fixed-white star colour,
// just now chosen per star instead of hardcoded.
#define ENABLE_FACE_COLOR 1

#define COLOR_FACE_WHITE   RGB565(255, 255, 255)
// Muted, not pure-saturated - a large flat-filled area in pure
// RGB565(255,0,0)-style colour on black reads as harsh/dated ("a 1970s
// video game"); mixing in some of the other channels softens each one
// toward a dustier tone without losing which is which.
#define COLOR_FACE_RED     RGB565(190, 90, 90)
#define COLOR_FACE_GREEN   RGB565(90, 160, 110)
#define COLOR_FACE_BLUE    RGB565(100, 130, 190)
#define COLOR_FACE_AMBER   RGB565(180, 160, 90)
#define COLOR_FACE_PURPLE  RGB565(140, 100, 160)
#define COLOR_FACE_TEAL    RGB565(90, 150, 150)
#define COLOR_FACE_ORANGE  RGB565(190, 130, 90)
#define COLOR_FACE_MAGENTA RGB565(170, 100, 140)

// Index 0-9 for a scene entry's 'color' field: 0=black (indistinguishable
// from the background - effectively "no colour"), 1=white, 2-9=the muted
// colours above. A static const array, not more #defines, so shape_color()
// in shapes.c can look an index up directly instead of a 10-way switch.
static const unsigned short FACE_COLOR_PALETTE[10] = {
    COLOR_BLACK,      COLOR_FACE_WHITE,
    COLOR_FACE_RED,   COLOR_FACE_GREEN,  COLOR_FACE_BLUE,
    COLOR_FACE_AMBER, COLOR_FACE_PURPLE, COLOR_FACE_TEAL, COLOR_FACE_ORANGE, COLOR_FACE_MAGENTA,
};

// --- Indexed colour framebuffer (experimental) ---------------------------
// This project only ever draws ~13 distinct colours total (FACE_COLOR_PALETTE's
// 10 plus the 3 axis colours) - RGB565's 65536 colours were always far more
// than needed. When on, both gfx_desktop.c and gfx_picocalc.c store one
// BYTE per pixel (a palette index - see color_palette.h) instead of one
// RGB565 unsigned short, halving the framebuffer from ~200KB to ~100KB.
// Expanding back to real RGB565 happens only at present/blit time (the
// portable core - renderer.c, hidden_line.c, etc. - never sees an index,
// only ever calls gfx_set_pixel() with a real RGB565 value, unchanged).
// The point of this is RAM, not speed: the LCD controller only understands
// RGB565 over SPI, so the same amount of data still has to go out over the
// wire either way - see gfx_picocalc.c's gfx_present() for how it expands
// a chunk at a time into a small scratch buffer rather than needing a
// second full-size buffer. Set to 0 and rebuild to go back to a plain
// RGB565 framebuffer on both platforms.
#define ENABLE_INDEXED_COLOR 1

// --- Reduced hidden-line depth precision (experimental) -------------------
// Halves hidden_line.c's occluder depth buffer from 16-bit (65535
// quantization levels) to 8-bit (255 levels) - like ENABLE_INDEXED_COLOR,
// this is purely about RAM (also ~200KB -> ~100KB), tried together with it
// specifically to see whether an RP2040 (264KB SRAM total) could fit both
// buffers at once (100KB + 100KB = 200KB, with headroom) where it
// previously couldn't fit either at full 16-bit size alongside the other.
// Real risk, not just a smaller number: each quantization level now
// represents roughly (HIDDEN_LINE_FAR_PLANE - NEAR_PLANE) / HIDDEN_LINE_DEPTH_MAX
// world units - about 15 units at 8-bit vs 0.06 at 16-bit - so two
// surfaces within that distance of each other in depth can round to the
// same level and fail to sort correctly. Most likely to show up as
// flicker right at a curved shape's own silhouette edge (the icosahedron
// sphere is the one to watch). Set to 0 and rebuild to go back to full
// 16-bit depth precision - HIDDEN_LINE_DEPTH_BIAS_QUANTIZED and
// HIDDEN_LINE_DEPTH_BIAS_QINVZ below both scale automatically either way.
#define ENABLE_REDUCED_DEPTH_PRECISION 1

#if ENABLE_REDUCED_DEPTH_PRECISION
#define HIDDEN_LINE_DEPTH_MAX 255
#else
#define HIDDEN_LINE_DEPTH_MAX 65535
#endif

// --- Wireframe shapes ---------------------------------------------------------
// World units are scaled up 100x from the project's original values (radius
// 6 -> 600, etc.) purely so the on-screen viewpoint coordinate readout (see
// the Status display section below) shows meaningfully varying integers
// instead of rounding tiny single-digit values to near-nothing. Perspective
// projection is invariant under uniformly scaling world distances and the
// camera's orbit radius together (the scale factor cancels out of the x/z
// and y/z ratios), so nothing on screen looks any different because of
// this - only the numbers in the status display do.
// The scene itself (every object, including stars - see shapes.c) is a
// plain, hand-edited table at the top of shapes.c, not generated here -
// see its comment for how to add or move one. Each entry also picks its
// own size (a scale multiplier - see shapes.c) rather than a shared fixed
// size. SHAPE_HALF_SIZE is the one geometry constant shared by every mesh
// type before that per-entry scale is applied (each mesh's local geometry
// spans -SHAPE_HALF_SIZE..+SHAPE_HALF_SIZE per axis at scale 1.0, or a
// multiple of it for an elongated shape like the house or the line - see
// mesh_definitions.h).
#define SHAPE_HALF_SIZE 80.0f

// --- Camera / orbit --------------------------------------------------------
// The camera always looks at the origin, moving over the surface of a
// sphere as the arrow keys are held. Its radius starts at ORBIT_RADIUS and
// can be changed live with F1 (zoom in) / F2 (zoom out) - see
// camera_zoom() - clamped to [ORBIT_RADIUS_MIN, ORBIT_RADIUS_MAX] so the
// camera can never fly inside a shape (MIN) or push shapes past
// HIDDEN_LINE_FAR_PLANE's depth range (MAX - see that constant's comment).
// AXIS_LENGTH is independent of ORBIT_RADIUS on purpose - the axis markers
// stop well short of the viewpoint rather than reaching out to it.
#define ORBIT_RADIUS 1875.0f
#define ORBIT_RADIUS_MIN 1100.0f
#define ORBIT_RADIUS_MAX 2800.0f
#define AXIS_LENGTH 250.0f      // shortened 75% from this project's original 1000.0f
#define ORBIT_TURN_SPEED_DEG_PER_SEC 90.0f
#define ORBIT_ZOOM_SPEED_UNITS_PER_SEC 600.0f

#define FOCAL_LENGTH 320.0f     // perspective projection scale (world-scale-independent, see above)
#define NEAR_PLANE 50.0f        // points/lines closer than this to the camera are clipped

// --- Status display (viewpoint coordinate readout, lower-right corner) -----
#define STATUS_MARGIN 8        // px from the right/bottom screen edges
#define STATUS_LINE_HEIGHT 18  // px between the X/Y/Z readout lines

// --- Hidden-line removal (hidden_line.h) ------------------------------------
// A per-pixel occluder depth buffer (~200KB at 16-bit, ~100KB at 8-bit -
// see ENABLE_REDUCED_DEPTH_PRECISION above) hides edges - or parts of
// edges - behind an opaque shape face, whether it belongs to a different
// shape or the same shape's own far side. Toggle at runtime with H in case
// it turns out too slow on real hardware (see input.h's toggle_hidden_line).
// HIDDEN_LINE_FAR_PLANE only needs to comfortably cover this scene's extent
// at the camera's farthest zoom-out (ORBIT_RADIUS_MAX plus the farthest a
// shape or axis endpoint can be from the origin), not be universally
// correct for any possible scene or camera distance.
#define HIDDEN_LINE_FAR_PLANE 4000.0f

// World-space depth slack for the occlusion test, so a line lying right on
// its own shape's front face isn't spuriously culled by small numeric
// differences between how draw_line() and hidden_line.c's triangle
// rasterizer each compute depth at the same screen pixel (two different
// interpolation paths that only agree approximately, not exactly). Kept
// much smaller than a shape's own depth extent (2 * SHAPE_HALF_SIZE = 160)
// so real shape-vs-shape occlusion still works correctly.
#define HIDDEN_LINE_DEPTH_BIAS_UNITS 10.0f
#define HIDDEN_LINE_DEPTH_BIAS_QUANTIZED ((int)(HIDDEN_LINE_DEPTH_BIAS_UNITS * (float)HIDDEN_LINE_DEPTH_MAX / (HIDDEN_LINE_FAR_PLANE - NEAR_PLANE)))

// Same idea as HIDDEN_LINE_DEPTH_BIAS_QUANTIZED, but for the fixed-point
// path's depth buffer, which stores a quantized 1/z (see hidden_line.c)
// instead of a quantized z - 1/z isn't linear in z, so there's no exact
// unit conversion between the two biases, just a similarly-sized empirical
// value - originally tuned as 200 on a 0..65535 scale, expressed here so
// it scales proportionally with HIDDEN_LINE_DEPTH_MAX instead of silently
// becoming a far bigger (or, at 8-bit, far too small) fraction of the
// range if that changes.
#define HIDDEN_LINE_DEPTH_BIAS_QINVZ ((int)(200.0f * (float)HIDDEN_LINE_DEPTH_MAX / 65535.0f))

// --- Timing ------------------------------------------------------------------
// main.c's loop sleeps only what's left of this budget after input+render
// actually take (see main()) - not a flat FRAME_MS on top of that work - so
// this is a ~30 FPS TARGET, not a guarantee: on real PicoCalc hardware,
// render+blit can exceed it, in which case that frame sleeps 0 and the real
// frame rate is whatever that work allows, slower than this implies.
#define FRAME_MS 33

// --- Input (PicoCalc backend only) -------------------------------------------
// The PicoCalc keyboard driver delivers held-key events roughly every 100ms
// (KEY_STATE_HOLD auto-repeat - see picocalc/drivers/keyboard.c). This many
// frames of silence must pass before input_picocalc.c treats a direction as
// released, so orbiting/zooming stays smooth at our own FRAME_MS instead of
// visibly stepping in 100ms jumps.
#define PICOCALC_KEY_HOLD_TIMEOUT_FRAMES 4

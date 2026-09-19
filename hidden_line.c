#include "hidden_line.h"
#include "constants.h"
#include "gfx.h"
#include <math.h>
#include <string.h>

#if USE_FIXED_POINT_MATH
#include "fixed.h"
#endif

/* depth_t is unsigned char (8-bit, 255 quantization levels) when
   ENABLE_REDUCED_DEPTH_PRECISION is on, unsigned short (16-bit, 65535
   levels) otherwise - see that flag's comment in constants.h for the RAM
   vs. precision tradeoff. HIDDEN_LINE_DEPTH_MAX (also constants.h) is the
   matching maximum value either way, so every quantization formula below
   scales itself automatically instead of hardcoding 65535. */
#if ENABLE_REDUCED_DEPTH_PRECISION
typedef unsigned char depth_t;
#else
typedef unsigned short depth_t;
#endif

static depth_t depth_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
static int enabled = 1;

void hidden_line_set_enabled(int e)
{
    enabled = e ? 1 : 0;
}

int hidden_line_enabled(void)
{
    return enabled;
}

void hidden_line_toggle(void)
{
    enabled = !enabled;
}

#define HIDDEN_LINE_MAX_POLYGON_VERTICES 8

#if USE_FIXED_POINT_MATH

/* Fixed-point path: the depth buffer stores a quantized 1/z instead of a
   quantized z, specifically so nothing in the per-pixel hot path (the
   triangle rasterizer, or draw_line()'s depth test) ever needs a division.
   1/z is what's affine in screen space under perspective (see the
   float-path comments below for why that matters for interpolation), and
   it's already monotonic in "closeness" - no need to invert it back to z
   just to compare two depths - so this path never does that inversion at
   all. "Closer" now means a LARGER stored value (more 1/z = smaller z),
   the opposite of the float path's "smaller wins"; hidden_line_begin_frame()
   and the comparisons below are written for that. The one division this
   scale still costs is computing 1/z itself, once per VERTEX in
   hidden_line_quantize_view_z() / project_for_hidden_line() - not the hot
   path this experiment targets (see USE_FIXED_POINT_MATH's comment in
   constants.h). */

#define HIDDEN_LINE_INV_Z_MIN (1.0f / HIDDEN_LINE_FAR_PLANE)
#define HIDDEN_LINE_INV_Z_MAX (1.0f / NEAR_PLANE)

static int project_for_hidden_line(const camera_t *cam, vec3_t world_point, fixed_t *sx, fixed_t *sy, int *qinvz);
static int64_t edge_function_raw(fixed_t ax, fixed_t ay, fixed_t bx, fixed_t by, fixed_t cx, fixed_t cy);
static void rasterize_triangle(fixed_t x0, fixed_t y0, int q0,
                                fixed_t x1, fixed_t y1, int q1,
                                fixed_t x2, fixed_t y2, int q2,
                                int fill, unsigned short fill_color);

void hidden_line_begin_frame(void)
{
    memset(depth_buffer, 0, sizeof depth_buffer);   /* 0 = "nothing occluding here yet" (farthest) */
}

int hidden_line_quantize_view_z(float z)
{
    float inv_z = 1.0f / z;
    float t = (inv_z - HIDDEN_LINE_INV_Z_MIN) / (HIDDEN_LINE_INV_Z_MAX - HIDDEN_LINE_INV_Z_MIN);

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return (int)(t * (float)HIDDEN_LINE_DEPTH_MAX);
}

int hidden_line_pixel_visible(int x, int y, int qinvz)
{
    if (!enabled) return 1;
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return 0;

    return qinvz + HIDDEN_LINE_DEPTH_BIAS_QINVZ >= (int)depth_buffer[y][x];
}

/* Same near-plane bail-out reasoning as the float path (see below), plus
   hidden_line_quantize_view_z()'s one division per vertex. */
// Author: Thomas Dzubin
static int project_for_hidden_line(const camera_t *cam, vec3_t world_point, fixed_t *sx, fixed_t *sy, int *qinvz)
{
    vec3_t v = camera_world_to_view(cam, world_point);
    fixed_t fx, fy, fz, focal;

    if (v.z <= NEAR_PLANE) return 0;

    fx = fixed_from_float(v.x);
    fy = fixed_from_float(v.y);
    fz = fixed_from_float(v.z);
    focal = fixed_from_float(FOCAL_LENGTH);

    /* fixed_muldiv(), not fixed_mul()+fixed_div(): v.x*FOCAL_LENGTH is a
       world-scale-times-320 intermediate (easily in the hundreds of
       thousands), which would overflow a narrowed Q16.16 fixed_mul()
       result long before the division by fz brings it back down to a
       screen-sized value - see fixed.h. */
    *sx = fixed_from_float(SCREEN_WIDTH / 2.0f) + fixed_muldiv(fx, focal, fz);
    *sy = fixed_from_float(SCREEN_HEIGHT / 2.0f) - fixed_muldiv(fy, focal, fz);
    *qinvz = hidden_line_quantize_view_z(v.z);
    return 1;
}

// Author: Thomas Dzubin
void hidden_line_rasterize_occluder_polygon(const camera_t *cam, const vec3_t *vertices, int vertex_count,
                                             int fill, unsigned short fill_color)
{
    fixed_t sx[HIDDEN_LINE_MAX_POLYGON_VERTICES], sy[HIDDEN_LINE_MAX_POLYGON_VERTICES];
    int q[HIDDEN_LINE_MAX_POLYGON_VERTICES];
    int i;

    if (!enabled) return;
    if (vertex_count > HIDDEN_LINE_MAX_POLYGON_VERTICES) vertex_count = HIDDEN_LINE_MAX_POLYGON_VERTICES;

    for (i = 0; i < vertex_count; i++) {
        if (!project_for_hidden_line(cam, vertices[i], &sx[i], &sy[i], &q[i])) return;
    }

    for (i = 1; i < vertex_count - 1; i++) {
        rasterize_triangle(sx[0], sy[0], q[0], sx[i], sy[i], q[i], sx[i + 1], sy[i + 1], q[i + 1],
                            fill, fill_color);
    }
}

/* Deliberately not fixed_mul(): for screen-scale coordinate differences
   (tens of pixels for this scene's shapes, but this isn't bounds-checked
   against anything larger) a narrowed Q16.16 product can itself overflow.
   Kept as a raw, unnarrowed int64 product instead (scaled by 2^32 relative
   to real units) - the scale cancels out cleanly in the w/area ratio in
   rasterize_triangle() below, so nothing downstream needs to know about
   it. */
static int64_t edge_function_raw(fixed_t ax, fixed_t ay, fixed_t bx, fixed_t by, fixed_t cx, fixed_t cy)
{
    int64_t t1 = (int64_t)(cx - ax) * (int64_t)(by - ay);
    int64_t t2 = (int64_t)(cy - ay) * (int64_t)(bx - ax);
    return t1 - t2;
}

/* Same edge-function rasterizer as the float path, but the per-vertex
   depth (q0/q1/q2, already quantized by project_for_hidden_line()) is
   barycentric-interpolated directly, with no per-pixel division:
   quantizing depth is itself an affine (positive-linear) map of 1/z, and
   an affine map of something that's affine in screen space is still affine
   in screen space, so blending the quantized values with these same
   weights is exactly as correct as blending 1/z itself would be. */
// Author: Thomas Dzubin
static void rasterize_triangle(fixed_t x0, fixed_t y0, int q0,
                                fixed_t x1, fixed_t y1, int q1,
                                fixed_t x2, fixed_t y2, int q2,
                                int fill, unsigned short fill_color)
{
    int64_t area = edge_function_raw(x0, y0, x1, y1, x2, y2);
    int min_x, max_x, min_y, max_y, x, y;

    if (area == 0) return;   /* degenerate (edge-on) triangle */

    min_x = fixed_to_int(x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2));
    max_x = fixed_to_int(x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2)) + 1;
    min_y = fixed_to_int(y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2));
    max_y = fixed_to_int(y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2)) + 1;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > SCREEN_WIDTH) max_x = SCREEN_WIDTH;
    if (max_y > SCREEN_HEIGHT) max_y = SCREEN_HEIGHT;

    for (y = min_y; y < max_y; y++) {
        for (x = min_x; x < max_x; x++) {
            fixed_t px = fixed_from_int(x) + FIXED_ONE / 2;
            fixed_t py = fixed_from_int(y) + FIXED_ONE / 2;
            int64_t w0 = edge_function_raw(x1, y1, x2, y2, px, py);
            int64_t w1 = edge_function_raw(x2, y2, x0, y0, px, py);
            int64_t w2 = edge_function_raw(x0, y0, x1, y1, px, py);

            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                int64_t q = (w0 * (int64_t)q0 + w1 * (int64_t)q1 + w2 * (int64_t)q2) / area;

                if (q > depth_buffer[y][x]) {
                    depth_buffer[y][x] = (depth_t)q;
                    if (fill) gfx_set_pixel(x, y, fill_color);
                }
            }
        }
    }
}

#else /* !USE_FIXED_POINT_MATH */

static depth_t quantize_depth(float z);
static int project_for_hidden_line(const camera_t *cam, vec3_t world_point, float *sx, float *sy, float *inv_z);
static float edge_function(float ax, float ay, float bx, float by, float cx, float cy);
static void rasterize_triangle(float x0, float y0, float invz0,
                                float x1, float y1, float invz1,
                                float x2, float y2, float invz2,
                                int fill, unsigned short fill_color);

void hidden_line_begin_frame(void)
{
    /* Every byte 0xFF -> every depth_t entry equal to HIDDEN_LINE_DEPTH_MAX
       (0xFF = 255 for an 8-bit depth_t, 0xFFFF = 65535 for 16-bit - either
       way, the largest quantized depth), "nothing occluding here yet". */
    memset(depth_buffer, 0xFF, sizeof depth_buffer);
}

static depth_t quantize_depth(float z)
{
    float t = (z - NEAR_PLANE) / (HIDDEN_LINE_FAR_PLANE - NEAR_PLANE);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return (depth_t)(t * (float)HIDDEN_LINE_DEPTH_MAX);
}

int hidden_line_pixel_visible(int x, int y, float z)
{
    int zq;

    if (!enabled) return 1;
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return 0;

    zq = (int)quantize_depth(z);
    return zq <= (int)depth_buffer[y][x] + HIDDEN_LINE_DEPTH_BIAS_QUANTIZED;
}

/* Projects a world point for occluder rasterization: screen (x, y) as
   sub-pixel floats (for accurate triangle coverage) plus 1/view-z - used
   instead of z itself because 1/z is affine in screen space for a
   perspective-projected planar triangle (so it interpolates correctly
   across the triangle) while z itself is not. Returns 0 if the point is at
   or behind NEAR_PLANE. */
static int project_for_hidden_line(const camera_t *cam, vec3_t world_point, float *sx, float *sy, float *inv_z)
{
    vec3_t v = camera_world_to_view(cam, world_point);
    if (v.z <= NEAR_PLANE) return 0;

    *sx = SCREEN_WIDTH / 2.0f + (v.x * FOCAL_LENGTH) / v.z;
    *sy = SCREEN_HEIGHT / 2.0f - (v.y * FOCAL_LENGTH) / v.z;
    *inv_z = 1.0f / v.z;
    return 1;
}

// Author: Thomas Dzubin
void hidden_line_rasterize_occluder_polygon(const camera_t *cam, const vec3_t *vertices, int vertex_count,
                                             int fill, unsigned short fill_color)
{
    float sx[HIDDEN_LINE_MAX_POLYGON_VERTICES], sy[HIDDEN_LINE_MAX_POLYGON_VERTICES], siz[HIDDEN_LINE_MAX_POLYGON_VERTICES];
    int i;

    if (!enabled) return;
    if (vertex_count > HIDDEN_LINE_MAX_POLYGON_VERTICES) vertex_count = HIDDEN_LINE_MAX_POLYGON_VERTICES;

    /* Faces this close to the camera never happen in this scene - shapes
       sit far inside the camera's orbit radius, see HIDDEN_LINE_FAR_PLANE's
       comment in constants.h - so bailing out on the whole polygon if any
       corner fails the near-plane test is simpler than clipping it, and in
       practice never triggers. */
    for (i = 0; i < vertex_count; i++) {
        if (!project_for_hidden_line(cam, vertices[i], &sx[i], &sy[i], &siz[i])) return;
    }

    /* Fan triangulation from vertex 0 - valid because every face here is
       convex and planar (see shapes.h), regardless of vertex count. */
    for (i = 1; i < vertex_count - 1; i++) {
        rasterize_triangle(sx[0], sy[0], siz[0],
                            sx[i], sy[i], siz[i],
                            sx[i + 1], sy[i + 1], siz[i + 1],
                            fill, fill_color);
    }
}

static float edge_function(float ax, float ay, float bx, float by, float cx, float cy)
{
    return (cx - ax) * (by - ay) - (cy - ay) * (bx - ax);
}

/* Standard edge-function triangle rasterizer: for every pixel in the
   triangle's screen-space bounding box, barycentric weights (w0, w1, w2)
   say whether the pixel center is inside (all same sign as the other two -
   this works for either winding order) and, when it is, interpolate 1/z to
   get this pixel's depth, keeping the closer of that and whatever's
   already in the depth buffer. */
// Author: Thomas Dzubin
static void rasterize_triangle(float x0, float y0, float invz0,
                                float x1, float y1, float invz1,
                                float x2, float y2, float invz2,
                                int fill, unsigned short fill_color)
{
    float area = edge_function(x0, y0, x1, y1, x2, y2);
    int min_x, max_x, min_y, max_y, x, y;

    if (area == 0.0f) return;   /* degenerate (edge-on) triangle */

    min_x = (int)floorf(x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2));
    max_x = (int)ceilf (x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2));
    min_y = (int)floorf(y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2));
    max_y = (int)ceilf (y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2));

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > SCREEN_WIDTH) max_x = SCREEN_WIDTH;
    if (max_y > SCREEN_HEIGHT) max_y = SCREEN_HEIGHT;

    for (y = min_y; y < max_y; y++) {
        for (x = min_x; x < max_x; x++) {
            float px = x + 0.5f, py = y + 0.5f;
            float w0 = edge_function(x1, y1, x2, y2, px, py);
            float w1 = edge_function(x2, y2, x0, y0, px, py);
            float w2 = edge_function(x0, y0, x1, y1, px, py);

            if ((w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) ||
                (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f)) {
                float b0 = w0 / area, b1 = w1 / area, b2 = w2 / area;
                float inv_z = b0 * invz0 + b1 * invz1 + b2 * invz2;
                depth_t zq = quantize_depth(1.0f / inv_z);

                if (zq < depth_buffer[y][x]) {
                    depth_buffer[y][x] = zq;
                    if (fill) gfx_set_pixel(x, y, fill_color);
                }
            }
        }
    }
}

#endif /* USE_FIXED_POINT_MATH */

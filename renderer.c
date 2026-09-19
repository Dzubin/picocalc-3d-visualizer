#include "renderer.h"
#include "gfx.h"
#include "constants.h"
#include "shapes.h"
#include "hidden_line.h"
#include "text.h"
#include "vec3.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#if USE_FIXED_POINT_MATH
#include "fixed.h"
#endif

static void rasterize_occluders(const camera_t *cam);
static vec3_t polygon_centroid(const vec3_t *vertices, int vertex_count);
static void draw_axis(const camera_t *cam, vec3_t dir, unsigned short color);
static void draw_shapes(const camera_t *cam);
static void draw_stars(const camera_t *cam);
static void draw_star(const camera_t *cam, vec3_t world_point, unsigned short color);
static void draw_world_segment(const camera_t *cam, vec3_t world_v0, vec3_t world_v1, unsigned short color);
static void draw_position_readout(const camera_t *cam);
static void draw_readout_line(const char *label, float value, int y, unsigned short color);
static void draw_hidden_line_status(void);

// Author: Thomas Dzubin
void renderer_draw_frame(const camera_t *cam)
{
    gfx_clear(COLOR_BLACK);

    if (hidden_line_enabled()) {
        hidden_line_begin_frame();
        rasterize_occluders(cam);
    }

    draw_axis(cam, vec3_make(1.0f, 0.0f, 0.0f), COLOR_AXIS_X);
    draw_axis(cam, vec3_make(0.0f, 1.0f, 0.0f), COLOR_AXIS_Y);
    draw_axis(cam, vec3_make(0.0f, 0.0f, 1.0f), COLOR_AXIS_Z);

    draw_shapes(cam);
    draw_stars(cam);

    draw_position_readout(cam);
    draw_hidden_line_status();

    gfx_present();
}

static vec3_t polygon_centroid(const vec3_t *vertices, int vertex_count)
{
    vec3_t sum = vec3_make(0.0f, 0.0f, 0.0f);
    int i;

    for (i = 0; i < vertex_count; i++) {
        sum = vec3_add(sum, vertices[i]);
    }
    return vec3_scale(sum, 1.0f / (float)vertex_count);
}

/* Rasterizes every shape's front-facing faces (the ones that could occlude
   something behind them) into hidden_line.c's depth buffer. "Front-facing"
   is: the camera sits on the outward side of the face's plane - true
   regardless of the face's vertex count or the shape's type. */
// Author: Thomas Dzubin
static void rasterize_occluders(const camera_t *cam)
{
    vec3_t cam_pos = camera_position(cam);
    int s, f;

    for (s = 0; s < shape_count(); s++) {
        unsigned short fill_color;

        if (shape_is_star(s)) continue;
        fill_color = shape_color(s);

        for (f = 0; f < shape_face_count(s); f++) {
            vec3_t vertices[SHAPE_MAX_FACE_VERTICES], normal, face_center;
            int vertex_count;

            shape_face(s, f, vertices, &vertex_count, &normal);
            face_center = polygon_centroid(vertices, vertex_count);

            if (vec3_dot(normal, vec3_sub(cam_pos, face_center)) > 0.0f) {
                hidden_line_rasterize_occluder_polygon(cam, vertices, vertex_count,
                                                        ENABLE_FACE_COLOR, fill_color);
            }
        }
    }
}

#if USE_FIXED_POINT_MATH

/* Projects a view-space point (z already known to be past NEAR_PLANE) to
   sub-pixel screen coordinates in Q16.16 fixed-point. fixed_muldiv(), not
   fixed_mul()+fixed_div(): see hidden_line.c's project_for_hidden_line()
   for why - the same overflow risk applies here. */
static void project_view_point(vec3_t v, fixed_t *sx, fixed_t *sy)
{
    fixed_t fx = fixed_from_float(v.x);
    fixed_t fy = fixed_from_float(v.y);
    fixed_t fz = fixed_from_float(v.z);
    fixed_t focal = fixed_from_float(FOCAL_LENGTH);

    *sx = fixed_from_float(SCREEN_WIDTH / 2.0f) + fixed_muldiv(fx, focal, fz);
    *sy = fixed_from_float(SCREEN_HEIGHT / 2.0f) - fixed_muldiv(fy, focal, fz);
}

/* A star has no faces of its own to self-occlude, so unlike a shape's
   edges it only ever needs the one hidden_line_pixel_visible() check
   against whatever shapes already wrote to the occluder buffer this frame -
   that alone is enough for a shape to correctly hide the stars behind
   it. */
// Author: Thomas Dzubin
static void draw_star(const camera_t *cam, vec3_t world_point, unsigned short color)
{
    vec3_t v = camera_world_to_view(cam, world_point);
    fixed_t sx, sy;
    int x, y, qinvz;

    if (v.z <= NEAR_PLANE) return;

    project_view_point(v, &sx, &sy);
    x = fixed_to_int(sx + FIXED_ONE / 2);
    y = fixed_to_int(sy + FIXED_ONE / 2);
    qinvz = hidden_line_quantize_view_z(v.z);

    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT && hidden_line_pixel_visible(x, y, qinvz)) {
        gfx_set_pixel(x, y, color);
    }
}

static void draw_line(fixed_t fx0, fixed_t fy0, int q0, fixed_t fx1, fixed_t fy1, int q1, unsigned short color);

/* Clips a world-space segment's endpoints against NEAR_PLANE (in view
   space, still float - see constants.h's USE_FIXED_POINT_MATH comment for
   why the view-space transform itself stays out of scope for this
   experiment) and draws it. Shared by the axis lines and the shape edges. */
// Author: Thomas Dzubin
static void draw_world_segment(const camera_t *cam, vec3_t world_v0, vec3_t world_v1, unsigned short color)
{
    vec3_t v0 = camera_world_to_view(cam, world_v0);
    vec3_t v1 = camera_world_to_view(cam, world_v1);
    fixed_t x0, y0, x1, y1;

    if (v0.z <= NEAR_PLANE && v1.z <= NEAR_PLANE) return;

    if (v0.z <= NEAR_PLANE) {
        float t = (NEAR_PLANE - v0.z) / (v1.z - v0.z);
        v0 = vec3_add(v0, vec3_scale(vec3_sub(v1, v0), t));
    } else if (v1.z <= NEAR_PLANE) {
        float t = (NEAR_PLANE - v1.z) / (v0.z - v1.z);
        v1 = vec3_add(v1, vec3_scale(vec3_sub(v0, v1), t));
    }

    project_view_point(v0, &x0, &y0);
    project_view_point(v1, &x1, &y1);
    draw_line(x0, y0, hidden_line_quantize_view_z(v0.z), x1, y1, hidden_line_quantize_view_z(v1.z), color);
}

#else /* !USE_FIXED_POINT_MATH */

/* Projects a view-space point (z already known to be past NEAR_PLANE) to
   sub-pixel screen coordinates - kept as floats, not rounded, so
   draw_line() can compute each pixel's depth from the true geometry
   instead of from already-rounded endpoints (see draw_line()'s comment on
   why that distinction matters for hidden-line removal). */
static void project_view_point(vec3_t v, float *sx, float *sy)
{
    *sx = SCREEN_WIDTH / 2.0f + (v.x * FOCAL_LENGTH) / v.z;
    *sy = SCREEN_HEIGHT / 2.0f - (v.y * FOCAL_LENGTH) / v.z;
}

/* A star has no faces of its own to self-occlude, so unlike a shape's
   edges it only ever needs the one hidden_line_pixel_visible() check
   against whatever shapes already wrote to the occluder buffer this frame -
   that alone is enough for a shape to correctly hide the stars behind
   it. */
// Author: Thomas Dzubin
static void draw_star(const camera_t *cam, vec3_t world_point, unsigned short color)
{
    vec3_t v = camera_world_to_view(cam, world_point);
    float sx, sy;
    int x, y;

    if (v.z <= NEAR_PLANE) return;

    project_view_point(v, &sx, &sy);
    x = (int)floorf(sx + 0.5f);
    y = (int)floorf(sy + 0.5f);

    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT && hidden_line_pixel_visible(x, y, v.z)) {
        gfx_set_pixel(x, y, color);
    }
}

static void draw_line(float fx0, float fy0, float z0, float fx1, float fy1, float z1, unsigned short color);

/* Clips a world-space segment's endpoints against NEAR_PLANE (in view
   space) and draws it. Shared by the axis lines and the shape edges - a
   straight 3D line still projects to a straight 2D line under perspective
   projection, so clipping only ever needs to fix up the two endpoints,
   never points in between. */
// Author: Thomas Dzubin
static void draw_world_segment(const camera_t *cam, vec3_t world_v0, vec3_t world_v1, unsigned short color)
{
    vec3_t v0 = camera_world_to_view(cam, world_v0);
    vec3_t v1 = camera_world_to_view(cam, world_v1);
    float x0, y0, x1, y1;

    if (v0.z <= NEAR_PLANE && v1.z <= NEAR_PLANE) return;

    if (v0.z <= NEAR_PLANE) {
        float t = (NEAR_PLANE - v0.z) / (v1.z - v0.z);
        v0 = vec3_add(v0, vec3_scale(vec3_sub(v1, v0), t));
    } else if (v1.z <= NEAR_PLANE) {
        float t = (NEAR_PLANE - v1.z) / (v0.z - v1.z);
        v1 = vec3_add(v1, vec3_scale(vec3_sub(v0, v1), t));
    }

    project_view_point(v0, &x0, &y0);
    project_view_point(v1, &x1, &y1);
    draw_line(x0, y0, v0.z, x1, y1, v1.z, color);
}

#endif /* USE_FIXED_POINT_MATH */

/* Draws one axis as a line from -AXIS_LENGTH to +AXIS_LENGTH along dir,
   through the origin. */
static void draw_axis(const camera_t *cam, vec3_t dir, unsigned short color)
{
    draw_world_segment(cam, vec3_scale(dir, -AXIS_LENGTH), vec3_scale(dir, AXIS_LENGTH), color);
}

static void draw_stars(const camera_t *cam)
{
    int s;

    for (s = 0; s < shape_count(); s++) {
        if (!shape_is_star(s)) continue;
        draw_star(cam, shape_center(s), shape_color(s));
    }
}

/* Draws every solid object's edges (stars are skipped - see draw_stars()),
   hiding ones that same object's own geometry hides (self-occlusion): an
   edge is only ever visible if at least one of its two adjacent faces (see
   shape_edge_faces()) faces the camera - the classic rule for a single
   convex solid, and one that works the same way regardless of the
   object's face/edge count. A side with no face at all (MESH_NO_FACE - see
   shapes.h; only OBJECT_LINE has one, on both sides of its one edge) can
   never occlude anything, so it always counts as "visible from that side"
   rather than indexing into front[] with it. Cross-shape occlusion is
   handled separately, per pixel, inside draw_line() via
   hidden_line_pixel_visible(). Both are skipped together when hidden-line
   removal is disabled, so "off" matches the pre-hidden-line-removal
   behaviour exactly. */
// Author: Thomas Dzubin
static void draw_shapes(const camera_t *cam)
{
    vec3_t cam_pos = camera_position(cam);
    int use_hidden_line = hidden_line_enabled();
    int s, e;

    for (s = 0; s < shape_count(); s++) {
        int front[SHAPE_MAX_FACES];

        if (shape_is_star(s)) continue;

        if (use_hidden_line) {
            int f;
            for (f = 0; f < shape_face_count(s); f++) {
                vec3_t vertices[SHAPE_MAX_FACE_VERTICES], normal, face_center;
                int vertex_count;
                shape_face(s, f, vertices, &vertex_count, &normal);
                face_center = polygon_centroid(vertices, vertex_count);
                front[f] = vec3_dot(normal, vec3_sub(cam_pos, face_center)) > 0.0f;
            }
        }

        for (e = 0; e < shape_edge_count(s); e++) {
            vec3_t v0, v1;

            if (use_hidden_line) {
                int face0, face1, visible0, visible1;
                shape_edge_faces(s, e, &face0, &face1);
                visible0 = (face0 == MESH_NO_FACE) || front[face0];
                visible1 = (face1 == MESH_NO_FACE) || front[face1];
                if (!visible0 && !visible1) continue;
            }

            shape_edge(s, e, &v0, &v1);
            draw_world_segment(cam, v0, v1, COLOR_SHAPE);
        }
    }
}

#if USE_FIXED_POINT_MATH

/* Bresenham's line algorithm (stepping between the endpoints rounded to
   the nearest pixel), clipped per-pixel against the screen and (via
   hidden_line_pixel_visible()) against the occluder depth buffer - same
   shape as the float version below, but with the one division per line
   (the reciprocal of the line's length-squared) precomputed once before
   the per-pixel loop instead of dividing inside it. That precompute is the
   actual point of converting this function: draw_line() runs once per
   pixel along every visible edge on screen, easily thousands of times a
   frame, so moving the division from per-PIXEL to per-LINE matters far
   more than fixed-vs-float does on its own - this same precompute would
   help the float path too, it's just not part of this experiment (see
   constants.h's USE_FIXED_POINT_MATH comment). */
// Author: Thomas Dzubin
static void draw_line(fixed_t fx0, fixed_t fy0, int q0, fixed_t fx1, fixed_t fy1, int q1, unsigned short color)
{
    int x0 = fixed_to_int(fx0 + FIXED_ONE / 2);
    int y0 = fixed_to_int(fy0 + FIXED_ONE / 2);
    int x1 = fixed_to_int(fx1 + FIXED_ONE / 2);
    int y1 = fixed_to_int(fy1 + FIXED_ONE / 2);
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    fixed_t dirx = fx1 - fx0, diry = fy1 - fy0;
    int64_t len_sq_raw = (int64_t)dirx * dirx + (int64_t)diry * diry;
    fixed_t inv_len_sq = 0;

    if (len_sq_raw > 0) {
        fixed_t len_sq = (fixed_t)(len_sq_raw >> FIXED_SHIFT);
        if (len_sq > 0) inv_len_sq = fixed_div(FIXED_ONE, len_sq);
    }

    for (;;) {
        fixed_t t = 0;

        if (inv_len_sq != 0) {
            int64_t num_raw = (int64_t)(fixed_from_int(x0) - fx0) * dirx +
                               (int64_t)(fixed_from_int(y0) - fy0) * diry;
            fixed_t num = (fixed_t)(num_raw >> FIXED_SHIFT);
            t = fixed_mul(num, inv_len_sq);
            if (t < 0) t = 0;
            if (t > FIXED_ONE) t = FIXED_ONE;
        }

        {
            int q = q0 + (int)(((int64_t)(q1 - q0) * t) >> FIXED_SHIFT);

            if (x0 >= 0 && x0 < SCREEN_WIDTH && y0 >= 0 && y0 < SCREEN_HEIGHT &&
                hidden_line_pixel_visible(x0, y0, q)) {
                gfx_set_pixel(x0, y0, color);
            }
        }
        if (x0 == x1 && y0 == y1) break;
        {
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

#else /* !USE_FIXED_POINT_MATH */

/* Bresenham's line algorithm (stepping between the endpoints rounded to
   the nearest pixel), clipped per-pixel against the screen and (via
   hidden_line_pixel_visible()) against the occluder depth buffer.

   Each stepped pixel's depth is found by projecting it back onto the
   TRUE, unrounded (fx0,fy0)-(fx1,fy1) segment (standard closest-point-on-
   a-segment formula) to get t, then interpolating 1/z by that t - not by
   Bresenham step index / step count. That distinction matters here: an
   earlier version used step-index-based t against endpoints already
   rounded to integers, which put its depth estimate measurably out of
   step with hidden_line.c's occluder rasterizer (which evaluates depth
   from the triangle's true sub-pixel vertex positions at the exact pixel
   center). For an edge lying right on its own shape's front face - the
   common case - those two independently-computed depths should be nearly
   identical, but the mismatch was occasionally enough to fail the depth
   test, breaking fully-visible edges into dashes. Reprojecting onto the
   true segment removes most of that error at the source;
   HIDDEN_LINE_DEPTH_BIAS_UNITS covers what's left. */
// Author: Thomas Dzubin
static void draw_line(float fx0, float fy0, float z0, float fx1, float fy1, float z1, unsigned short color)
{
    int x0 = (int)floorf(fx0 + 0.5f);
    int y0 = (int)floorf(fy0 + 0.5f);
    int x1 = (int)floorf(fx1 + 0.5f);
    int y1 = (int)floorf(fy1 + 0.5f);
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    float inv_z0 = 1.0f / z0, inv_z1 = 1.0f / z1;
    float dirx = fx1 - fx0, diry = fy1 - fy0;
    float len_sq = dirx * dirx + diry * diry;

    for (;;) {
        float t = 0.0f;
        if (len_sq > 0.0f) {
            t = ((x0 - fx0) * dirx + (y0 - fy0) * diry) / len_sq;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
        }

        {
            float z = 1.0f / (inv_z0 + (inv_z1 - inv_z0) * t);

            if (x0 >= 0 && x0 < SCREEN_WIDTH && y0 >= 0 && y0 < SCREEN_HEIGHT &&
                hidden_line_pixel_visible(x0, y0, z)) {
                gfx_set_pixel(x0, y0, color);
            }
        }
        if (x0 == x1 && y0 == y1) break;
        {
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

#endif /* USE_FIXED_POINT_MATH */

/* The viewpoint's world-space X/Y/Z, as whole numbers, stacked in the
   lower-right corner - each line colour-matched to its axis. */
static void draw_position_readout(const camera_t *cam)
{
    vec3_t pos = camera_position(cam);
    int y = SCREEN_HEIGHT - STATUS_MARGIN - 3 * STATUS_LINE_HEIGHT;

    draw_readout_line("X:", pos.x, y, COLOR_AXIS_X);
    draw_readout_line("Y:", pos.y, y + STATUS_LINE_HEIGHT, COLOR_AXIS_Y);
    draw_readout_line("Z:", pos.z, y + 2 * STATUS_LINE_HEIGHT, COLOR_AXIS_Z);
}

/* Whether hidden-line removal is currently on, in the top-right corner -
   toggled with H (see input.h's toggle_hidden_line). */
static void draw_hidden_line_status(void)
{
    char line[24];

    snprintf(line, sizeof line, "H KEY=HLR:%d", hidden_line_enabled());
    text_draw(SCREEN_WIDTH - STATUS_MARGIN - text_width(line), STATUS_MARGIN, line, COLOR_STATUS);
}

static void draw_readout_line(const char *label, float value, int y, unsigned short color)
{
    char line[16];
    int rounded = (int)(value >= 0.0f ? value + 0.5f : value - 0.5f);

    snprintf(line, sizeof line, "%s%d", label, rounded);
    text_draw(SCREEN_WIDTH - STATUS_MARGIN - text_width(line), y, line, color);
}

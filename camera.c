#include "camera.h"
#include "constants.h"
#include "shapes.h"
#include <math.h>

#define DEG2RAD(d) ((d) * 3.14159265358979323846f / 180.0f)

/* Below this length the cross product of the line of sight and the world's
   up is too short to give a direction (looking almost straight up or down),
   so the camera's own right vector is used for strafing instead. */
#define STRAFE_DEGENERATE_LENGTH 0.05f

static void rotate_pair(vec3_t *a, vec3_t *b, float angle);
static void keep_at_right_angles(camera_t *cam);
static void keep_inside_flight_limits(camera_t *cam);

void camera_init(camera_t *cam)
{
    camera_reset(cam);
}

// Author: Thomas Dzubin
void camera_reset(camera_t *cam)
{
    float az = DEG2RAD(CAMERA_START_AZIMUTH_DEG);
    float el = DEG2RAD(CAMERA_START_ELEVATION_DEG);
    float horiz = CAMERA_START_DISTANCE * cosf(el);

    cam->position = vec3_make(horiz * sinf(az), CAMERA_START_DISTANCE * sinf(el), horiz * cosf(az));
    cam->forward = vec3_normalize(vec3_scale(cam->position, -1.0f));
    cam->right = vec3_make(cosf(az), 0.0f, -sinf(az));
    cam->up = vec3_cross(cam->right, cam->forward);
}

/* Turns a pair of the camera's axes towards each other by angle (radians):
   a becomes a cos + b sin and b becomes b cos - a sin. Every turn below is
   this on a different pair, so the third axis (the one being turned about)
   is untouched. */
static void rotate_pair(vec3_t *a, vec3_t *b, float angle)
{
    float c = cosf(angle), s = sinf(angle);
    vec3_t new_a = vec3_add(vec3_scale(*a, c), vec3_scale(*b, s));
    vec3_t new_b = vec3_sub(vec3_scale(*b, c), vec3_scale(*a, s));

    *a = new_a;
    *b = new_b;
}

// Author: Thomas Dzubin
void camera_turn(camera_t *cam, float pitch_deg, float yaw_deg, float roll_deg)
{
    /* Pitch is about right: forward tilts towards up. Yaw is about up:
       forward turns towards right. Roll is about forward: right and up turn
       with it, right tipping down (a roll to the right), hence the minus. */
    if (pitch_deg != 0.0f) rotate_pair(&cam->forward, &cam->up, DEG2RAD(pitch_deg));
    if (yaw_deg != 0.0f)   rotate_pair(&cam->forward, &cam->right, DEG2RAD(yaw_deg));
    if (roll_deg != 0.0f)  rotate_pair(&cam->right, &cam->up, -DEG2RAD(roll_deg));

    keep_at_right_angles(cam);
}

/* Puts forward, up and right back at exact right angles and unit length (a
   Gram-Schmidt step). Each turn is exact in theory, but a float turn is a
   little off every time, and the errors would add up over many frames. */
static void keep_at_right_angles(camera_t *cam)
{
    cam->forward = vec3_normalize(cam->forward);
    cam->up = vec3_normalize(vec3_sub(cam->up, vec3_scale(cam->forward, vec3_dot(cam->up, cam->forward))));
    cam->right = vec3_cross(cam->forward, cam->up);
}

// Author: Thomas Dzubin
void camera_fly(camera_t *cam, float forward_units, float strafe_units, float rise_units)
{
    vec3_t world_up = vec3_make(0.0f, 1.0f, 0.0f);
    vec3_t side = vec3_cross(cam->forward, world_up);

    /* The sideways direction comes from the line of sight and the world's
       up only, never from the camera's own right vector, because that one
       tips when the camera rolls. */
    if (vec3_length(side) < STRAFE_DEGENERATE_LENGTH) {
        side = vec3_make(cam->right.x, 0.0f, cam->right.z);
    }
    side = vec3_normalize(side);

    cam->position = vec3_add(cam->position, vec3_scale(cam->forward, forward_units));
    cam->position = vec3_add(cam->position, vec3_scale(side, strafe_units));
    cam->position = vec3_add(cam->position, vec3_scale(world_up, rise_units));

    keep_inside_flight_limits(cam);
}

/* The flight boundary: a sphere around the origin, and a keep-out sphere
   around every solid object. A camera that has gone past one is pushed
   straight back to its surface, which makes it slide along the edge instead
   of sticking. Two passes, so that being pushed out of one object into the
   next settles. */
// Author: Thomas Dzubin
static void keep_inside_flight_limits(camera_t *cam)
{
    float distance = vec3_length(cam->position);
    int pass, s;

    if (distance > CAMERA_BOUNDARY_RADIUS) {
        cam->position = vec3_scale(cam->position, CAMERA_BOUNDARY_RADIUS / distance);
    }

    for (pass = 0; pass < 2; pass++) {
        for (s = 0; s < shape_count(); s++) {
            vec3_t away;
            float keep_out, d;

            if (shape_is_star(s)) continue;

            keep_out = shape_bounding_radius(s) + CAMERA_SHAPE_CLEARANCE;
            away = vec3_sub(cam->position, shape_center(s));
            d = vec3_length(away);
            if (d >= keep_out) continue;

            if (d < 1e-3f) away = vec3_make(1.0f, 0.0f, 0.0f);
            else away = vec3_scale(away, 1.0f / d);
            cam->position = vec3_add(shape_center(s), vec3_scale(away, keep_out));
        }
    }
}

vec3_t camera_position(const camera_t *cam)
{
    return cam->position;
}

vec3_t camera_world_to_view(const camera_t *cam, vec3_t world_point)
{
    vec3_t rel = vec3_sub(world_point, cam->position);

    return vec3_make(vec3_dot(rel, cam->right), vec3_dot(rel, cam->up), vec3_dot(rel, cam->forward));
}

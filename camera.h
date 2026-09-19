//
//  camera.h - an orbit camera that always looks at the origin from a
//  distance (orbit_radius, starting at ORBIT_RADIUS - see constants.h),
//  moved by changing its azimuth and elevation angles (arrow keys) or its
//  orbit radius (F1/F2 - see camera_zoom()). No platform dependencies.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"

typedef struct {
    float azimuth_deg;     // rotation around the Y axis, wraps 0-360
    float elevation_deg;   // angle above/below the XZ plane, wraps 0-360 (passes through both poles)
    float orbit_radius;    // distance from the origin - starts at ORBIT_RADIUS, adjustable live with camera_zoom()

    /* position/right/up/forward are derived from azimuth_deg/elevation_deg/
       orbit_radius - recomputed once by camera_init()/camera_orbit()/
       camera_zoom() (the only functions that change those), not on every
       camera_position()/camera_world_to_view() call. Those recompute trig,
       and with several hundred projected points per frame (every shape
       edge/face vertex), recomputing per-call instead of per-frame was
       real, measurable waste. Treat these as read-only outside camera.c. */
    vec3_t position;
    vec3_t right;
    vec3_t up;
    vec3_t forward;
} camera_t;

void camera_init(camera_t *cam, float azimuth_deg, float elevation_deg);

// Adjusts the camera's position on the orbit sphere by the given deltas.
void camera_orbit(camera_t *cam, float d_azimuth_deg, float d_elevation_deg);

// Moves the camera closer to (negative delta_units) or further from
// (positive delta_units) the origin, clamped to [ORBIT_RADIUS_MIN,
// ORBIT_RADIUS_MAX] (see constants.h) so it can never fly inside a shape or
// push the scene past hidden_line.c's far plane.
void camera_zoom(camera_t *cam, float delta_units);

// The camera's world-space position on the orbit sphere.
vec3_t camera_position(const camera_t *cam);

// Transforms a world-space point into view space: X = right, Y = up,
// Z = distance in front of the camera (the camera always faces the origin).
vec3_t camera_world_to_view(const camera_t *cam, vec3_t world_point);

#endif

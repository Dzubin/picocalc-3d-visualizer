//
//  camera.h - a free-flight camera. It has a position and three unit
//  vectors that say which way it is facing (forward, up and right), and it
//  is moved the way an aircraft is: pitch, yaw and roll turn it about its
//  own axes (camera_turn()), and camera_fly() moves it. The flight is kept
//  inside a boundary sphere and out of the solid objects. No platform
//  dependencies.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"

typedef struct {
    /* Read-only outside camera.c. right, up and forward are unit vectors at
       right angles to each other (up = right x forward). They are kept as
       vectors, not as angles, so the camera can
       pitch straight over the top or roll any amount with no pole or gimbal
       lock; camera_turn() puts them back at right angles after every turn so
       small rounding errors do not build up. */
    vec3_t position;
    vec3_t right;
    vec3_t up;
    vec3_t forward;
} camera_t;

// Puts the camera at its starting place (CAMERA_START_* in constants.h),
// looking at the origin with the world's up as its up.
void camera_init(camera_t *cam);

// The same as camera_init(): the Z key.
void camera_reset(camera_t *cam);

// Turns the camera about its own axes, in degrees. Positive pitch tilts the
// view up (nose up), positive yaw turns it right, and positive roll banks it
// right, so the picture turns counter-clockwise on the screen.
void camera_turn(camera_t *cam, float pitch_deg, float yaw_deg, float roll_deg);

// Moves the camera. forward_units goes along the line of sight (negative is
// backwards), strafe_units sideways and rise_units straight up (negative is
// down). Sideways and up are in the world's frame, not the camera's, so
// rolling never changes where the arrow keys take you: sideways is the
// direction at right angles to the line of sight on the world's ground plane,
// up is the world's +Y. The camera is then held inside the flight boundary
// (CAMERA_BOUNDARY_RADIUS) and outside every solid object.
void camera_fly(camera_t *cam, float forward_units, float strafe_units, float rise_units);

// The camera's world-space position.
vec3_t camera_position(const camera_t *cam);

// Transforms a world-space point into view space: X = right, Y = up,
// Z = distance in front of the camera.
vec3_t camera_world_to_view(const camera_t *cam, vec3_t world_point);

#endif

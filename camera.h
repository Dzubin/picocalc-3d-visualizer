//
//  camera.h - a camera that moves over the surface of a sphere and faces its
//  centre. The camera has a position and three unit vectors that say which
//  way it is facing (forward, up and right), plus the sphere's radius. The
//  centre of the sphere is the point straight ahead of the camera at that
//  radius, so it is not fixed: turning the camera with camera_turn() moves
//  it. camera_orbit() moves the camera over the sphere, camera_zoom() changes
//  the radius. No platform dependencies.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"

typedef struct {
    /* Read-only outside camera.c. right, up and forward are unit vectors at
       right angles to each other (up = right x forward). They are kept as
       vectors, not as angles, so the camera can pitch or orbit straight over
       the top or roll any amount with no pole or gimbal lock; they are put
       back at right angles after every change so small rounding errors do
       not build up. radius is the distance from the camera to the centre of
       the sphere. side is the horizontal axis that the UP and DOWN arrows
       turn about (see camera_orbit()). */
    vec3_t position;
    vec3_t right;
    vec3_t up;
    vec3_t forward;
    vec3_t side;
    float radius;
} camera_t;

// Puts the camera at its starting place (CAMERA_START_* in constants.h),
// looking at the origin with the world's up as its up.
void camera_init(camera_t *cam);

// The same as camera_init(): the Z key.
void camera_reset(camera_t *cam);

// The centre of the sphere the camera is on: the point straight ahead of it,
// radius away.
vec3_t camera_center(const camera_t *cam);

// Turns the camera about its own axes where it is, in degrees. Positive pitch
// tilts the view up (nose up), positive yaw turns it right, and positive roll
// banks it right, so the picture turns counter-clockwise on the screen. Pitch
// and yaw swing the centre of the sphere about the camera; roll does not move
// it.
void camera_turn(camera_t *cam, float pitch_deg, float yaw_deg, float roll_deg);

// Moves the camera over the surface of its sphere, in degrees, still facing
// the centre. Positive azimuth goes round the world's vertical axis through
// the centre (the RIGHT arrow); positive elevation goes up over the top of the
// sphere (the UP arrow), through the pole and down the other side if you keep
// going. Both are in the world's frame, so a roll never changes where the
// arrow keys take you. The camera is then held inside the flight boundary
// (CAMERA_BOUNDARY_RADIUS) and outside every solid object, and if one of
// those moved it, the centre moves with it.
void camera_orbit(camera_t *cam, float azimuth_deg, float elevation_deg);

// Changes the sphere's radius by delta_units (negative is closer), keeping the
// centre where it is, between CAMERA_RADIUS_MIN and CAMERA_RADIUS_MAX.
void camera_zoom(camera_t *cam, float delta_units);

// The camera's world-space position.
vec3_t camera_position(const camera_t *cam);

// Transforms a world-space point into view space: X = right, Y = up,
// Z = distance in front of the camera.
vec3_t camera_world_to_view(const camera_t *cam, vec3_t world_point);

#endif

#include "camera.h"
#include "constants.h"
#include <math.h>

#define DEG2RAD(d) ((d) * 3.14159265358979323846f / 180.0f)

static void update_derived(camera_t *cam);

void camera_init(camera_t *cam, float azimuth_deg, float elevation_deg)
{
    cam->azimuth_deg = azimuth_deg;
    cam->elevation_deg = elevation_deg;
    cam->orbit_radius = ORBIT_RADIUS;
    update_derived(cam);
}

// Author: Thomas Dzubin
void camera_orbit(camera_t *cam, float d_azimuth_deg, float d_elevation_deg)
{
    cam->azimuth_deg += d_azimuth_deg;
    if (cam->azimuth_deg >= 360.0f) cam->azimuth_deg -= 360.0f;
    if (cam->azimuth_deg < 0.0f) cam->azimuth_deg += 360.0f;

    /* Elevation wraps the same as azimuth rather than clamping - see
       update_derived()'s comment for why that still traces a sane path
       through the poles instead of degenerating. */
    cam->elevation_deg += d_elevation_deg;
    if (cam->elevation_deg >= 360.0f) cam->elevation_deg -= 360.0f;
    if (cam->elevation_deg < 0.0f) cam->elevation_deg += 360.0f;

    update_derived(cam);
}

void camera_zoom(camera_t *cam, float delta_units)
{
    cam->orbit_radius += delta_units;
    if (cam->orbit_radius < ORBIT_RADIUS_MIN) cam->orbit_radius = ORBIT_RADIUS_MIN;
    if (cam->orbit_radius > ORBIT_RADIUS_MAX) cam->orbit_radius = ORBIT_RADIUS_MAX;

    update_derived(cam);
}

/* Recomputes position/right/up/forward from azimuth_deg/elevation_deg/
   orbit_radius. Called once per camera_init()/camera_orbit()/camera_zoom()
   (i.e. at most once per frame - main.c calls camera_orbit() every frame
   regardless of whether any arrow key is actually held), not once per
   camera_position()/camera_world_to_view() call - those are read-only
   lookups now. */
// Author: Thomas Dzubin
static void update_derived(camera_t *cam)
{
    float az = DEG2RAD(cam->azimuth_deg);
    float el = DEG2RAD(cam->elevation_deg);
    /* horiz turns negative once elevation passes 90 degrees, which flips
       both x and z below - equivalent to azimuth+180 - so this still
       traces a continuous great circle up over the north pole, down the
       far meridian, under the south pole, and back, instead of getting
       stuck at either pole. */
    float horiz = cam->orbit_radius * cosf(el);

    cam->position = vec3_make(horiz * sinf(az), cam->orbit_radius * sinf(el), horiz * cosf(az));
    cam->forward = vec3_normalize(vec3_scale(cam->position, -1.0f));
    /* "right" is the horizontal tangent of the orbit sphere at this
       azimuth, independent of elevation - it is always exactly
       perpendicular to forward (forward's horizontal component always
       points along this same azimuth, by construction above), so it stays
       smooth and well-defined straight through both poles. Deriving it
       from cross(forward, world_up) instead degenerates exactly at a pole,
       and patching that with a reference-axis swap near the pole (an
       earlier version of this function did that) just moves the
       singularity to the swap threshold instead of removing it - visible
       as a jump right where the swap kicked in. */
    cam->right = vec3_make(cosf(az), 0.0f, -sinf(az));
    cam->up = vec3_cross(cam->right, cam->forward);
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

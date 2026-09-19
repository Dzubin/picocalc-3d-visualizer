//
//  vec3.h - minimal portable 3D vector math. No platform dependencies.
//

#ifndef VEC3_H
#define VEC3_H

#include <math.h>

typedef struct {
    float x, y, z;
} vec3_t;

static inline vec3_t vec3_make(float x, float y, float z)
{
    vec3_t v;
    v.x = x; v.y = y; v.z = z;
    return v;
}

static inline vec3_t vec3_add(vec3_t a, vec3_t b)
{
    return vec3_make(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline vec3_t vec3_sub(vec3_t a, vec3_t b)
{
    return vec3_make(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline vec3_t vec3_scale(vec3_t a, float s)
{
    return vec3_make(a.x * s, a.y * s, a.z * s);
}

static inline float vec3_dot(vec3_t a, vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3_t vec3_cross(vec3_t a, vec3_t b)
{
    return vec3_make(a.y * b.z - a.z * b.y,
                      a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x);
}

static inline float vec3_length(vec3_t a)
{
    return sqrtf(vec3_dot(a, a));
}

static inline vec3_t vec3_normalize(vec3_t a)
{
    float len = vec3_length(a);
    if (len < 1e-6f) {
        return vec3_make(0.0f, 0.0f, 0.0f);
    }
    return vec3_scale(a, 1.0f / len);
}

#endif

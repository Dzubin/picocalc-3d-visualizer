//
//  fixed.h - Q16.16 signed fixed-point arithmetic (16 integer bits, 16
//  fractional bits, stored in a 32-bit int), for the experimental
//  USE_FIXED_POINT_MATH hot-path renderer (see constants.h, renderer.c,
//  hidden_line.c). fixed_mul()/fixed_div() widen their operands to 64-bit before
//  the compensating shift/divide back down to 32-bit, so a screen-scale
//  intermediate product can't silently overflow the way it would staying
//  in 32-bit the whole way through - that widen-then-narrow step is the
//  actual point of using 64-bit integers here. fixed_muldiv() goes one
//  step further for the common "multiply two, then divide by a third"
//  pattern (e.g. the perspective divide): it never narrows the product at
//  all, only the final quotient, so it tolerates operands too large for
//  fixed_mul() to multiply safely. No platform dependencies.
//

#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

typedef int32_t fixed_t;

#define FIXED_SHIFT 16
#define FIXED_ONE ((fixed_t)1 << FIXED_SHIFT)

static inline fixed_t fixed_from_float(float v)
{
    return (fixed_t)(v * (float)FIXED_ONE);
}

static inline float fixed_to_float(fixed_t v)
{
    return (float)v / (float)FIXED_ONE;
}

static inline fixed_t fixed_from_int(int v)
{
    return (fixed_t)v << FIXED_SHIFT;
}

static inline int fixed_to_int(fixed_t v)
{
    return (int)(v >> FIXED_SHIFT);
}

static inline fixed_t fixed_mul(fixed_t a, fixed_t b)
{
    return (fixed_t)(((int64_t)a * (int64_t)b) >> FIXED_SHIFT);
}

static inline fixed_t fixed_div(fixed_t a, fixed_t b)
{
    return (fixed_t)(((int64_t)a << FIXED_SHIFT) / b);
}

// (a * b) / c, without narrowing the a*b product to 32 bits first - safe
// even when a and b are individually large enough that fixed_mul(a, b)
// alone would overflow, as long as the final quotient fits a fixed_t.
static inline fixed_t fixed_muldiv(fixed_t a, fixed_t b, fixed_t c)
{
    return (fixed_t)(((int64_t)a * (int64_t)b) / c);
}

#endif

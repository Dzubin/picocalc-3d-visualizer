//
//  timing.h - the one platform-specific timing call the portable core
//  needs. desktop/timing_desktop.c and picocalc/timing_picocalc.c each
//  implement it against their own OS/SDK.
//

#ifndef TIMING_H
#define TIMING_H

void platform_sleep_ms(int ms);

// Microseconds on some platform-specific monotonic clock. Only ever used to
// measure how long a frame's input+render work took (see main()'s frame
// pacing) via subtraction - that's safe under unsigned wraparound as long
// as no single measured interval exceeds the clock's wrap period, which a
// single frame never will.
unsigned long platform_now_us(void);

#endif

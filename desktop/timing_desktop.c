#include "../timing.h"
#include <SDL.h>

void platform_sleep_ms(int ms)
{
    SDL_Delay((Uint32)ms);
}

unsigned long platform_now_us(void)
{
    static Uint64 frequency = 0;
    if (frequency == 0) frequency = SDL_GetPerformanceFrequency();

    return (unsigned long)(SDL_GetPerformanceCounter() * 1000000ULL / frequency);
}

#include "../timing.h"
#include "pico/stdlib.h"
#include "pico/time.h"

void platform_sleep_ms(int ms)
{
    sleep_ms((uint32_t)ms);
}

unsigned long platform_now_us(void)
{
    return (unsigned long)time_us_64();
}

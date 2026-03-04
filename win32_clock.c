#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include "types.h"
static f64 clock_frequency;
static UINT min_period;
static LARGE_INTEGER start_time;

void clock_setup(void) {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(tc));
    min_period = tc.wPeriodMin;
}

f64 platform_get_absolute_time(void) {
    if (!clock_frequency) {
        clock_setup();
    }

    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}
#endif

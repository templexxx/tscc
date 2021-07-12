//
// Created by templex on 7/9/21.
//
#include "time.h"

#define TSC_ENABLED
#define TSC_FREQ

long long std_ns()
{
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

long long tsc_ns()
{
    return 0;
}

long long tsc_nsec()
{
#ifdef TSC_ENABLED
    return std_ns();
#else
    return tsc_ns();
#endif
}


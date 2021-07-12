//
// Created by templex on 7/9/21.
//
#include "time.h"
#include "stdlib.h"

#ifndef TSC_FREQ
#  define TSC_FREQ 1.0;
#endif

double tsc_coeff = 1 / (TSC_FREQ / 1e9);

static inline uint64_t rdtsc(void) {
    unsigned  cycles_low, cycles_high;
    asm volatile( "RDTSC\n\t"
                  "MOV %%edx, %0\n\t"
                  "MOV %%eax, %1\n\t"
    : "=r" (cycles_high), "=r" (cycles_low)
    :: "%rax", "%rbx", "%rcx", "%rdx" );
    return ((uint64_t) cycles_high << 32) | cycles_low;
}

long long tsc_std_ns(void) {
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

static void fast_calibrate(uint64_t *minDelta, uint64_t *tsc, uint64_t *wall) {
    // 256 is enough for finding lowest wall clock cost in most cases.
    // Although clock_gettime is using VDSO to get time, but it's unstable,
    // sometimes it will take more than 1000ns,
    // we have to use a big loop(e.g. 256) to get the "real" clock.
    // And it won't take a long time to finish the calibrate job, only about 20µs.
    //
    // [tsc, wc, tsc, wc, ..., tsc]
    uint64_t timeline[256+256+1];

    timeline[0] = rdtsc();
    int i;
    for (i = 1; i < 512; i += 2) {
        timeline[i] = (uint64_t) tsc_std_ns();
        timeline[i+1] = rdtsc();
    }

    // The minDelta is the smallest gap between two adjacent tscs,
    // which means the smallest gap between wall clock and tsc too.
    *minDelta = UINT64_MAX;
    int minIndex = 1;// minIndex is wall clock index where has minDelta.

    // clock_gettime's precision is only µs (on MacOS),
    // which means we will get multi same wall clock in timeline,
    // and the middle one is closer to the real time in statistics.
    // Try to find the minimum delta when wall clock is in the "middle".
    for (i = 1; i < 512; i += 2) {
        uint64_t last = timeline[i];
        int j;
        for (j = i + 2; j < 512; j += 2) {
            if (timeline[j] != last) {
                int mid = (i + j - 2) >> 1;
                if ((mid & 1) == 0) {
                    mid++;
                }

                uint64_t delta = timeline[mid+1] - timeline[mid-1];
                if (delta < *minDelta) {
                    *minDelta = delta;
                    minIndex = mid;
                }

                i = j;
                last = timeline[j];
            }
        }
    }

    *tsc = (timeline[minIndex+1] + timeline[minIndex-1]) >> 1;
    *wall = timeline[minIndex];
}

typedef union {
    long long offset;
    long long padding[16]; // 128Bytes padding avoiding false sharing.
} tsc_offset_block __attribute__((aligned(128)));

tsc_offset_block tsc_offset;

static void init_offset(void) __attribute__((constructor));

static void init_offset(void) {
#if TSC_ENABLED
    uint64_t min_delta, min_tsc, min_wall;
    min_delta = UINT64_MAX;
    int i;
    for (i = 0; i < 1024; i++) { // Try to find the best one.
        uint64_t md, tsc, wall;
        fast_calibrate(&md, &tsc, &wall);
        if (md < min_delta) {
            min_delta = md;
            min_tsc = tsc;
            min_wall = wall;
        }
    }
    uint64_t off = min_wall - (uint64_t) ((double) min_tsc * tsc_coeff);
    tsc_offset.offset = (long long) off;
#endif
}

inline long long tsc_tsc_ns(void) {

    return (long long) (rdtsc() * tsc_coeff) + tsc_offset.offset;
}

inline long long tsc_nsec(void) {
#if TSC_ENABLED
    return tsc_tsc_ns();
#else
    return tsc_std_ns();
#endif
}

void tsc_calibrate_offset(void) {
#if TSC_ENABLED
    uint64_t md, tsc, wall;
    fast_calibrate(&md, &tsc, &wall);
    uint64_t off = md - (uint64_t) ((double) tsc * tsc_coeff);
    tsc_offset.offset = (long long) off;
#endif
}
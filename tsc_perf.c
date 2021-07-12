//
// Created by templex on 7/10/21.
//

#include "tscc.h"
#include "stdio.h"

int main() {

    long n = 100000000;
    long long tmp;

    long long start = tsc_nsec();
    for (long i = 0; i < n; ++i) {
        tmp += tsc_nsec();
    }
    long long end = tsc_nsec();

    printf("tsc_nsec: %.2f ns/op\n", ((double) end-start) / (double) n);
    return 0;
}
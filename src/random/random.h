#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

typedef struct {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

extern pcg32_random_t pcg_state;

void pcg32_seed(uint64_t seed);
uint32_t pcg32_random(void);
double pcg32_random_double(void);

#endif
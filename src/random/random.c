#include "random.h"

pcg32_random_t pcg_state = {0, 0};

void pcg32_seed(const uint64_t seed) {
    pcg_state.state = seed + 0x853c49e6748fea9bULL;
    pcg_state.inc = (seed << 1u) | 1u;
}

uint32_t pcg32_random(void) {
    const uint64_t old_state = pcg_state.state;
    pcg_state.state = old_state * 6364136223846793005ULL + pcg_state.inc;
    const uint32_t xor_shifted = (uint32_t)(((old_state >> 18u) ^ old_state) >> 27u);
    const uint32_t rot = (uint32_t)(old_state >> 59u);
    return (xor_shifted >> rot) | (xor_shifted << ((-rot) & 31));
}

double pcg32_random_double(void) {
    return (double)pcg32_random() / (double)0x100000000ULL;
}
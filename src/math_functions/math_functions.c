#include "math_functions.h"

#include <math.h>

double math_sign(const double num) {
    if (num < 0) {
        return -1;
    }
    if (num > 0) {
        return 1;
    }
    return 0;
}

double math_xavier(const double fan_in, const double fan_out) {
    return sqrt(6.0 / (fan_in + fan_out));
}

double math_sigmoid(const double num) {
    if (num >= 0) {
        return 1.0 / (1.0 + exp(-num));
    }
    const double e = exp(num);
    return e / (1.0 + e);
}
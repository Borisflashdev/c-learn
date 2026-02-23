#include "math_functions.h"

#include <math.h>

double math_sign(const double x) {
    if (x < 0) {
        return -1;
    }
    if (x > 0) {
        return 1;
    }
    return 0;
}

double math_xavier(const double fan_in, const double fan_out) {
    return sqrt(6.0 / (fan_in + fan_out));
}

double math_sigmoid(const double x) {
    if (x >= 0) {
        return 1.0 / (1.0 + exp(-x));
    }
    const double e = exp(x);
    return e / (1.0 + e);
}

double math_relu(const double x) {
    return x > 0 ? x : 0;
}

double math_leaky_relu(const double x) {
    return x > 0 ? x : 0.01 * x;
}

double math_silu(const double x) {
    return x * math_sigmoid(x);
}

double math_tanh(const double x) {
    if (x >= 0) {
        const double e = exp(-2.0 * x);
        return (1.0 - e) / (1.0 + e);
    }
    const double e = exp(2.0 * x);
    return (e - 1.0) / (e + 1.0);
}

double math_derivative_relu(const double x) {
    return x > 0 ? 1 : 0;
}

double math_derivative_leaky_relu(const double x) {
    return x > 0 ? 1 : 0.01;
}

double math_derivative_silu(const double x) {
    return math_sigmoid(x) + x * math_derivative_sigmoid(x);
}

double math_derivative_sigmoid(const double x) {
    return math_sigmoid(x) * (1 - math_sigmoid(x));
}

double math_derivative_tanh(const double x) {
    return 1 - pow(math_tanh(x), 2);
}

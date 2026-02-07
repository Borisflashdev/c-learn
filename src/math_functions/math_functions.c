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

double math_xavier(const double num) {
    return sqrt(6 / (num + 1));
}
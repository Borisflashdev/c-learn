#include "math_functions.h"

double math_sign(const double num) {
    if (num < 0) {
        return -1;
    }
    if (num > 0) {
        return 1;
    }
    return 0;
}

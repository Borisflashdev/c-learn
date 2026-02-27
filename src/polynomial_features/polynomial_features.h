#ifndef POLYNOMIAL_FEATURES_H
#define POLYNOMIAL_FEATURES_H

#include "../matrix/matrix.h"

Matrix *polynomial_features(const Matrix *X, int degree);

#endif
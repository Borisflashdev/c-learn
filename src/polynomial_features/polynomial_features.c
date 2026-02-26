#include "polynomial_features.h"

Matrix *polynomial_features(const Matrix *X, const int degree) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (degree < 2) {
        CUSTOM_ERROR("'degree' must be in range (2, ...)");
        return NULL;
    }

    Matrix *res = matrix_create(X->rows, X->cols * degree);

    return res;
}
#include "polynomial_features.h"

#include <math.h>

Matrix *polynomial_features(const Matrix *X, const int degree) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (degree < 2) {
        CUSTOM_ERROR("'degree' must be in range (2, ...)");
        return NULL;
    }

    Matrix *pol = matrix_create(X->rows, X->cols * degree - X->cols);

    int index = 0;
    for (int p = 2; p <= degree; p++) {
        for (int j = 0; j < X->cols; j++) {
            for (int i = 0; i < X->rows; i++) {
                matrix_set(pol, i, index, pow(matrix_get(X, i, j), p));
            }
            index++;
        }
    }

    Matrix *res = matrix_concat(X, pol);
    matrix_free(pol);
    return res;
}

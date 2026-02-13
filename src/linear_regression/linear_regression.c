#include "linear_regression.h"

#include <math.h>
#include <stdlib.h>

LinearRegression *linear_regression_create(const int number_of_features, const int fit_intercept) {
    if (number_of_features < 1) {
        CUSTOM_ERROR("'number_of_features' must be at least 1");
        return NULL;
    }
    if (fit_intercept != 0 && fit_intercept != 1) {
        CUSTOM_ERROR("Property 'fit_intercept' must be 0 or 1");
        return NULL;
    }

    LinearRegression *lr = malloc(sizeof(LinearRegression));
    if (!lr) {
        ALLOCATION_ERROR();
        return NULL;
    }
    lr->coef = vector_create(number_of_features);
    if (!lr->coef) {
        ALLOCATION_ERROR();
        free(lr);
        return NULL;
    }
    lr->intercept = NAN;
    lr->lambda = NAN;
    lr->fit_intercept = fit_intercept;
    lr->number_of_features = number_of_features;

    return lr;
}

void linear_regression_free(LinearRegression *linear_regression) {
    if (!linear_regression) {
        NULL_ERROR("Linear regression model");
        return;
    }

    if (linear_regression->coef) {
        vector_free(linear_regression->coef);
    }
    free(linear_regression);
}

void linear_regression_fit(LinearRegression *model, Matrix *X, Vector *y, const double lambda) {
    if (!model) {
        NULL_ERROR("Linear regression model");
        return;
    }
    if (!X) {
        NULL_ERROR("Matrix");
        return;
    }
    if (!y) {
        NULL_ERROR("Vector");
        return;
    }
    if (X->rows != y->dim || X->cols != model->number_of_features) {
        CUSTOM_ERROR("X->rows must equal y->dim and X->cols must equal number_of_features");
        return;
    }

    const int n_features = X->cols;
    const int n_samples = X->rows;
    const int size = model->fit_intercept ? n_features + 1 : n_features;
    model->lambda = lambda;

    Matrix *A = matrix_create(size, size);
    Vector *b = vector_create(size);

    for (int i = 0; i < n_samples; i++) {
        for (int row = 0; row < size; row++) {
            double val_row;
            if (model->fit_intercept) {
                val_row = row == 0 ? 1.0 : matrix_get(X, i, row - 1);
            } else {
                val_row = matrix_get(X, i, row);
            }

            vector_set(b, row, vector_get(b, row) + val_row * vector_get(y, i));

            for (int col = row; col < size; col++) {
                double val_col;
                if (model->fit_intercept) {
                    val_col = col == 0 ? 1.0 : matrix_get(X, i, col - 1);
                } else {
                    val_col = matrix_get(X, i, col);
                }

                const double current_val = matrix_get(A, row, col);
                const double new_val = current_val + val_row * val_col;
                matrix_set(A, row, col, new_val);
                if (row != col) {
                    matrix_set(A, col, row, new_val);
                }
            }
        }
    }

    const int start_idx = model->fit_intercept ? 1 : 0;
    for (int i = start_idx; i < size; i++) {
        const double val = matrix_get(A, i, i);
        matrix_set(A, i, i, val + lambda);
    }

    Matrix *A_inv = matrix_inverse(A, 0);
    if (!A_inv) {
        CUSTOM_ERROR("Matrix is singular");
        matrix_free(A);
        vector_free(b);
        return;
    }

    if (model->fit_intercept) {
        double inter_val = 0;
        for (int j = 0; j < size; j++) inter_val += matrix_get(A_inv, 0, j) * vector_get(b, j);
        model->intercept = inter_val;

        for (int i = 1; i < size; i++) {
            double w_i = 0;
            for (int j = 0; j < size; j++) w_i += matrix_get(A_inv, i, j) * vector_get(b, j);
            vector_set(model->coef, i - 1, w_i);
        }
    } else {
        for (int i = 0; i < size; i++) {
            double w_i = 0;
            for (int j = 0; j < size; j++) w_i += matrix_get(A_inv, i, j) * vector_get(b, j);
            vector_set(model->coef, i, w_i);
        }
    }

    matrix_free(A);
    matrix_free(A_inv);
    vector_free(b);
}

Vector *linear_regression_predict(LinearRegression *model, Matrix *X) {
    if (!model) {
        NULL_ERROR("Linear regression model");
        return NULL;
    }
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (X->cols != model->number_of_features) {
        CUSTOM_ERROR("X->cols must equal number_of_features");
        return NULL;
    }

    Vector *res = vector_create(X->rows);
    for (int i = 0; i < res->dim; i++) {
        double dot = 0;
        for (int j = 0; j < model->coef->dim; j++) {
            dot += vector_get(model->coef, j) * matrix_get(X, i, j);
        }
        if (model->fit_intercept == 1) {
            vector_set(res, i, dot + model->intercept);
        } else {
            vector_set(res, i, dot);
        }
    }
    return res;
}
#include "sgdregression.h"

#include <stdlib.h>
#include <tgmath.h>
#include <time.h>

SGDRegression *sgd_regression_create(const int number_of_features, const int fit_intercept, const Penalty penalty) {
    if (number_of_features < 1) {
        INDEX_ERROR();
        return NULL;
    }
    if (fit_intercept != 0 && fit_intercept != 1) {
        CUSTOM_ERROR("Property 'fit_intercept' must be 0 or 1");
        return NULL;
    }
    SGDRegression *sgd = malloc(sizeof(SGDRegression));
    sgd->coef = vector_create(number_of_features);
    if (!sgd->coef) {
        ALLOCATION_ERROR();
        return NULL;
    }
    sgd->intercept = NAN;
    sgd->lambda = NAN;
    sgd->ratio = NAN;
    sgd->fit_intercept = fit_intercept;
    sgd->number_of_features = number_of_features;
    sgd->penalty = penalty;

    return sgd;
}

void sgd_regression_free(SGDRegression *model) {
    if (!model) {
        NULL_ERROR("SDRegression model");
        return;
    }
    if (model->coef) {
        vector_free(model->coef);
    }
    free(model);
}

void sgd_regression_fit(SGDRegression *model, Matrix *X, Vector *y, const double w_in, const double b_in, const double alpha, const int num_iters, const double lambda, const double ratio) {
    if (!model) {
        NULL_ERROR("SDRegression model");
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
        CUSTOM_ERROR("Dimension mismatch");
        return;
    }
    if (num_iters < 1) {
        CUSTOM_ERROR("Number of iterations must be greater than zero");
        return;
    }
    if (alpha < 0) {
        CUSTOM_ERROR("Alpha must be greater than zero");
        return;
    }

    switch (model->penalty) {
        case NO_PENALTY: {
            if (!isnan(lambda) || !isnan(ratio)) {
                CUSTOM_ERROR("lambda and ratio is not use with 'NO_PENALTY', must be set to NAN");
                return;
            }
            break;
        }
        case L1_LASSO: {
            if (!isnan(ratio)) {
                CUSTOM_ERROR("ratio is not use with 'L1_LASSO', must be set to NAN");
                return;
            }
            if (lambda < 0) {
                CUSTOM_ERROR("Lambda must be greater than zero");
                return;
            }
            break;
        }
        case L2_RIDGE: {
            if (!isnan(ratio)) {
                CUSTOM_ERROR("ratio is not use with 'L2_RIDGE', must be set to NAN");
                return;
            }
            if (lambda < 0) {
                CUSTOM_ERROR("Lambda must be greater than zero");
                return;
            }
            break;
        }
        case ELASTIC_NET: {
            if (lambda < 0) {
                CUSTOM_ERROR("Lambda must be greater than zero");
                return;
            }
            if (ratio < 0 || ratio > 1) {
                CUSTOM_ERROR("Ratio must be between 0 and 1");
                return;
            }
            break;
        }
    }

    const int m = X->rows;
    const int n = X->cols;

    for (int i = 0; i < model->number_of_features; i++) {
        vector_set(model->coef, i, w_in);
    }
    model->intercept = b_in;
    model->lambda = lambda;
    model->ratio = ratio;

    Vector *indices = vector_create(m);
    for (int i = 0; i < m; i++) {
        vector_set(indices, i, i);
    }
    srand(time(NULL));

    for (int iter = 0; iter < num_iters; iter++) {
        vector_shuffle(indices);

        for (int idx = 0; idx < m; idx++) {
            const int i = (int) vector_get(indices, idx);

            double dot = 0;
            for (int j = 0; j < n; j++) {
                dot += matrix_get(X, i, j) * vector_get(model->coef, j);
            }
            const double prediction = dot + model->intercept;
            const double error_i = prediction - vector_get(y, i);

            if (model->fit_intercept == 1) {
                model->intercept -= alpha * error_i;
            }

            for (int j = 0; j < n; j++) {
                const double xi_j = matrix_get(X, i, j);
                double w_j = vector_get(model->coef, j);

                const double grad_j = error_i * xi_j;

                switch (model->penalty) {
                    case NO_PENALTY: {
                        w_j -= alpha * grad_j;
                        break;
                    }
                    case L1_LASSO: {
                        w_j -= alpha * (grad_j + math_sign(w_j)*lambda);
                        break;
                    }
                    case L2_RIDGE: {
                        w_j -= alpha * (grad_j + 2*lambda*w_j);
                        break;
                    }
                    case ELASTIC_NET: {
                        const double penalty = lambda * (ratio * math_sign(w_j)+(1-ratio)*2*w_j);
                        w_j = w_j - alpha * (grad_j + penalty);
                        break;
                    }
                }
                vector_set(model->coef, j, w_j);
            }
        }
    }
    vector_free(indices);
}

Vector *sgd_regression_predict(SGDRegression *model, Matrix *X) {
    if (!model) {
        NULL_ERROR("SDRegression model");
        return NULL;
    }
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (X->cols != model->number_of_features) {
        CUSTOM_ERROR("Dimension mismatch");
        return NULL;
    }

    Matrix *w_mat = vector_to_matrix(model->coef);
    Matrix *y_hat = matrix_multiplication(X, w_mat);
    Vector *res = matrix_to_vector(y_hat, 0, 0, y_hat->rows);

    matrix_free(w_mat);
    matrix_free(y_hat);
    if (model->fit_intercept == 0) {
        return res;
    }

    for (int i = 0; i < res->dim; i++) {
        res->data[i] += model->intercept;
    }
    return res;
}
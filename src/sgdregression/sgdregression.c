#include "sgdregression.h"

#include <stdint.h>
#include <stdlib.h>
#include <tgmath.h>
#include <time.h>

SGDRegression *sgd_regression_create(const int number_of_features, const int fit_intercept, const int random_seed, const Penalty penalty) {
    if (number_of_features < 1) {
        CUSTOM_ERROR("'number_of_features' must be at least 1");
        return NULL;
    }
    if (fit_intercept != 0 && fit_intercept != 1) {
        CUSTOM_ERROR("Property 'fit_intercept' must be 0 or 1");
        return NULL;
    }
    SGDRegression *sgd = malloc(sizeof(SGDRegression));
    if (!sgd) {
        ALLOCATION_ERROR();
        return NULL;
    }
    sgd->coef = vector_create(number_of_features);
    if (!sgd->coef) {
        ALLOCATION_ERROR();
        free(sgd);
        return NULL;
    }
    sgd->intercept = NAN;
    sgd->lambda = NAN;
    sgd->ratio = NAN;
    sgd->fit_intercept = fit_intercept;
    sgd->number_of_features = number_of_features;
    sgd->random_seed = random_seed;
    sgd->penalty = penalty;

    return sgd;
}

void sgd_regression_free(SGDRegression *model) {
    if (!model) {
        NULL_ERROR("SGDRegression model");
        return;
    }
    if (model->coef) {
        vector_free(model->coef);
    }
    free(model);
}

void sgd_regression_fit(SGDRegression *model, Matrix *X, Vector *y, const int batch, const double alpha, const int num_iters, const double lambda, const double ratio, const int print_every) {
    if (!model) {
        NULL_ERROR("SGDRegression model");
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
    if (batch <= 0 || batch > y->dim) {
        CUSTOM_ERROR("batch must be between 1 and the number of samples");
        return;
    }
    if (X->rows != y->dim || X->cols != model->number_of_features) {
        CUSTOM_ERROR("X->rows must equal y->dim and X->cols must equal number_of_features");
        return;
    }
    if (num_iters < 1) {
        CUSTOM_ERROR("'num_iters' must be at least 1");
        return;
    }
    if (alpha < 0) {
        CUSTOM_ERROR("'alpha' must be non-negative");
        return;
    }
    if (print_every < 0) {
        CUSTOM_ERROR("'print_every' must be non-negative");
        return;
    }

    switch (model->penalty) {
        case NO_PENALTY: {
            if (!isnan(lambda) || !isnan(ratio)) {
                CUSTOM_ERROR("'lambda' and 'ratio' are unused with NO_PENALTY, pass NAN");
                return;
            }
            break;
        }
        case L1_LASSO: {
            if (!isnan(ratio)) {
                CUSTOM_ERROR("'ratio' is unused with L1_LASSO, pass NAN");
                return;
            }
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("'lambda' must be non-negative");
                return;
            }
            break;
        }
        case L2_RIDGE: {
            if (!isnan(ratio)) {
                CUSTOM_ERROR("'ratio' is unused with L2_RIDGE, pass NAN");
                return;
            }
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("'lambda' must be non-negative");
                return;
            }
            break;
        }
        case ELASTIC_NET: {
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("'lambda' must be non-negative");
                return;
            }
            if (ratio < 0 || ratio > 1 || isnan(ratio)) {
                CUSTOM_ERROR("'ratio' must be between 0 and 1");
                return;
            }
            break;
        }
    }

    const uint64_t seed = model->random_seed < 0 ? (uint64_t)time(NULL) : (uint64_t)model->random_seed;
    pcg32_seed(seed);

    const double limit = math_xavier(X->cols, 1);
    for (int i = 0; i < model->number_of_features; i++) {
        const double random_w = pcg32_random_double() * 2.0 * limit - limit;
        vector_set(model->coef, i, random_w);
    }
    model->intercept = 0;
    model->lambda = lambda;
    model->ratio = ratio;

    Vector *indices = vector_create(X->rows);
    for (int i = 0; i < indices->dim; i++) {
        vector_set(indices, i, i);
    }

    for (int iter = 0; iter < num_iters; iter++) {
        vector_shuffle(indices);
        double total_epoch_loss = 0;

        for (int k = 0; k < X->rows; k += batch) {
            const int current_batch_size = k + batch > X->rows ? X->rows - k : batch;

            Vector *grad_sums = vector_create(model->number_of_features);
            double intercept_grad_sum = 0;

            for (int i = 0; i < current_batch_size; i++) {
                const int row_idx = (int)vector_get(indices, k + i);

                double y_hat = 0;
                for (int j = 0; j < model->number_of_features; j++) {
                    y_hat += matrix_get(X, row_idx, j) * vector_get(model->coef, j);
                }
                if (model->fit_intercept) {
                    y_hat += model->intercept;
                }

                const double error = y_hat - vector_get(y, row_idx);
                total_epoch_loss += error * error;

                for (int j = 0; j < model->number_of_features; j++) {
                    vector_set(grad_sums, j, vector_get(grad_sums, j) + error * matrix_get(X, row_idx, j));
                }
                intercept_grad_sum += error;
            }

            for (int j = 0; j < model->number_of_features; j++) {
                const double grad_j = vector_get(grad_sums, j) / current_batch_size;
                double w_j = vector_get(model->coef, j);

                switch (model->penalty) {
                    case L2_RIDGE:
                        w_j -= alpha * (grad_j + lambda * w_j);
                        break;
                    case L1_LASSO:
                        w_j -= alpha * (grad_j + lambda * (w_j > 0 ? 1 : -1));
                        break;
                    case ELASTIC_NET: {
                        const double l1 = ratio * (w_j > 0 ? 1 : -1);
                        const double l2 = (1.0 - ratio) * w_j;
                        w_j -= alpha * (grad_j + lambda * (l1 + l2));
                        break;
                    }
                    default:
                        w_j -= alpha * grad_j;
                        break;
                }
                vector_set(model->coef, j, w_j);
            }

            if (model->fit_intercept) {
                model->intercept -= alpha * (intercept_grad_sum / current_batch_size);
            }
            vector_free(grad_sums);
        }

        if (print_every > 0 && (iter % print_every == 0 || iter == num_iters - 1)) {
            printf("Epoch: %d | Cost (MSE): [%lf]\n", iter + 1, total_epoch_loss / (2.0 * X->rows));
        }
    }
    vector_free(indices);
}

Vector *sgd_regression_predict(SGDRegression *model, Matrix *X) {
    if (!model) {
        NULL_ERROR("SGDRegression model");
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
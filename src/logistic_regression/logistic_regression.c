#include "logistic_regression.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

LogisticRegression *logistic_regression_create(const int number_of_features, const int fit_intercept, const int random_seed, const double threshold, const Penalty penalty) {
    if (number_of_features < 1) {
        CUSTOM_ERROR("'number_of_features' must be at least 1");
        return NULL;
    }
    if (fit_intercept != 0 && fit_intercept != 1) {
        CUSTOM_ERROR("Property 'fit_intercept' must be 0 or 1");
        return NULL;
    }
    LogisticRegression *lr = malloc(sizeof(LogisticRegression));
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
    lr->ratio = NAN;
    lr->fit_intercept = fit_intercept;
    lr->number_of_features = number_of_features;
    lr->random_seed = random_seed;
    lr->threshold = threshold;
    lr->penalty = penalty;

    return lr;
}

void logistic_regression_free(LogisticRegression *model) {
    if (!model) {
        NULL_ERROR("LogisticRegression model");
        return;
    }
    if (model->coef) {
        vector_free(model->coef);
    }
    free(model);
}

void logistic_regression_fit(LogisticRegression *model, Matrix *X, Vector *y, const int batch, const double alpha, const int num_iters, double lambda, double ratio, const int print_every) {
    if (!model) {
        NULL_ERROR("LogisticRegression model");
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

    Matrix *y_col = vector_to_matrix(y);
    Matrix *set = matrix_concat(X, y_col);
    matrix_free(y_col);

    for (int iter = 0; iter < num_iters; iter++) {
        Matrix *shuffle = matrix_shuffle_rows(set);
        double total_epoch_loss = 0;

        for (int k = 0; k < shuffle->rows; k += batch) {
            const int current_batch_size = k + batch > shuffle->rows ? shuffle->rows - k : batch;

            Vector *grad_sums = vector_create(model->number_of_features);
            double intercept_grad_sum = 0;

            for (int i = 0; i < current_batch_size; i++) {
                const int row_idx = k + i;

                double dot = 0;
                for (int j = 0; j < model->number_of_features; j++) {
                    dot += matrix_get(shuffle, row_idx, j) * vector_get(model->coef, j);
                }
                if (model->fit_intercept) {
                    dot += model->intercept;
                }

                const double y_hat = math_sigmoid(dot);
                const double error = y_hat - matrix_get(shuffle, row_idx, shuffle->cols - 1);

                const double eps = 1e-15;
                total_epoch_loss += -1 * matrix_get(shuffle, row_idx, shuffle->cols - 1) * log(y_hat + eps) - (1 - matrix_get(shuffle, row_idx, shuffle->cols - 1)) * log(1 - y_hat + eps);

                for (int j = 0; j < model->number_of_features; j++) {
                    vector_set(grad_sums, j, vector_get(grad_sums, j) + error * matrix_get(shuffle, row_idx, j));
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
            printf("Epoch: %d | Cost (LOSS): [%lf]\n", iter + 1, total_epoch_loss / shuffle->rows);
        }
        matrix_free(shuffle);
    }
    matrix_free(set);
}

Vector *logistic_regression_predict_proba(LogisticRegression *model, Matrix *X) {
    if (!model) {
        NULL_ERROR("LogisticRegression model");
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
    if (!res) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int i = 0; i < X->rows; i++) {
        double dot = 0;
        for (int j = 0; j < X->cols; j++) {
            dot += matrix_get(X, i, j) * vector_get(model->coef, j);
        }
        vector_set(res, i, model->fit_intercept == 0 ? math_sigmoid(dot) : math_sigmoid(dot+model->intercept));
    }
    return res;
}

Vector *logistic_regression_predict(LogisticRegression *model, Matrix *X) {
    Vector *res = logistic_regression_predict_proba(model, X);
    if (!res) {
        ALLOCATION_ERROR();
        return NULL;
    }
    for (int i = 0; i < res->dim; i++) {
        vector_set(res, i, vector_get(res, i) >= model->threshold ? 1 : 0);
    }
    return res;
}
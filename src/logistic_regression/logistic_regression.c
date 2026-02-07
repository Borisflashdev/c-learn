#include "logistic_regression.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

LogisticRegression *logistic_regression_create(const int number_of_features, const int fit_intercept, const int random_seed, const double threshold, const Penalty penalty) {
    if (number_of_features < 1) {
        INDEX_ERROR();
        return NULL;
    }
    if (fit_intercept != 0 && fit_intercept != 1) {
        CUSTOM_ERROR("Property 'fit_intercept' must be 0 or 1");
        return NULL;
    }
    LogisticRegression *lr = malloc(sizeof(LogisticRegression));
    lr->coef = vector_create(number_of_features);
    if (!lr->coef) {
        ALLOCATION_ERROR();
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

void logistic_regression_fit(LogisticRegression *model, Matrix *X, Vector *y, const double alpha, const int num_iters, double lambda, double ratio) {
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
            if (lambda < 0 || isnan(lambda)) {
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
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("Lambda must be greater than zero");
                return;
            }
            break;
        }
        case ELASTIC_NET: {
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("Lambda must be greater than zero");
                return;
            }
            if (ratio < 0 || ratio > 1 || isnan(ratio)) {
                CUSTOM_ERROR("Ratio must be between 0 and 1");
                return;
            }
            break;
        }
    }

    const int m = X->rows;
    const int n = X->cols;

    srand(model->random_seed < 0 ? time(NULL) : model->random_seed);

    const double limit = math_xavier(n);
    for (int i = 0; i < model->number_of_features; i++) {
        const double random_w = ((double)rand() / (double)RAND_MAX * 2.0 * limit) - limit;
        vector_set(model->coef, i, random_w);
    }
    model->intercept = 0;
    model->lambda = lambda;
    model->ratio = ratio;

    Vector *indices = vector_create(m);
    for (int i = 0; i < m; i++) {
        vector_set(indices, i, i);
    }

    for (int iter = 0; iter < num_iters; iter++) {
        vector_shuffle(indices);

        for (int idx = 0; idx < m; idx++) {
            const int i = (int) vector_get(indices, idx);

            double dot = 0;
            for (int j = 0; j < n; j++) {
                dot += matrix_get(X, i, j) * vector_get(model->coef, j);
            }
            const double prediction = math_sigmoid(dot + model->intercept);
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
                        w_j -= alpha * (grad_j + lambda*w_j);
                        break;
                    }
                    case ELASTIC_NET: {
                        const double penalty = lambda * (ratio * math_sign(w_j)+(1-ratio)*w_j);
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
        CUSTOM_ERROR("Dimension mismatch");
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
        if (model->fit_intercept == 0) {
            res->data[i] = math_sigmoid(dot);
        } else {
            res->data[i] = math_sigmoid(dot+model->intercept);
        }
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
        if (res->data[i] >= model->threshold) {
            res->data[i] = 1;
        } else {
            res->data[i] = 0;
        }
    }

    return res;
}
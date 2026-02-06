#include "bgdregression.h"

#include <stdlib.h>
#include <tgmath.h>

BGDRegression *bgd_regression_create(const int number_of_features, const int fit_intercept, const RegularizationType regularization_type) {
    if (number_of_features < 1) {
        INDEX_ERROR();
        return NULL;
    }
    if (fit_intercept != 0 && fit_intercept != 1) {
        CUSTOM_ERROR("Property 'fit_intercept' must be 0 or 1");
        return NULL;
    }
    BGDRegression *bgd = malloc(sizeof(BGDRegression));
    bgd->coef = vector_create(number_of_features);
    if (!bgd->coef) {
        ALLOCATION_ERROR();
        return NULL;
    }
    bgd->intercept = 0;
    bgd->lambda = 0;
    bgd->ratio = 0;
    bgd->fit_intercept = fit_intercept;
    bgd->number_of_features = number_of_features;
    bgd->regularization_type = regularization_type;

    return bgd;
}

void bgd_regression_free(BGDRegression *bgd_regression) {
    if (!bgd_regression) {
        NULL_ERROR("GDRegression model");
        return;
    }
    if (bgd_regression->coef) {
        vector_free(bgd_regression->coef);
    }
    free(bgd_regression);
}

void bgd_regression_fit(BGDRegression *model, Matrix *X, Vector *y, const double w_in, const double b_in, const double alpha, const int num_iters, const double lambda, const double ratio, const int print_iter) {
    if (!model) {
        NULL_ERROR("GDRegression model");
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
    if (print_iter != 0 && print_iter != 1) {
        CUSTOM_ERROR("Property 'print_iter' must be 0 or 1");
        return;
    }
    if (alpha < 0) {
        CUSTOM_ERROR("Alpha must be greater than zero");
        return;
    }
    if (lambda < 0) {
        CUSTOM_ERROR("Lambda must be greater than zero");
        return;
    }
    if (ratio < 0 || ratio > 1) {
        CUSTOM_ERROR("Ratio must be between 0 and 1");
        return;
    }

    const int m = X->rows;
    const int n = X->cols;

    for (int i = 0; i < model->number_of_features; i++) {
        vector_set(model->coef, i, w_in);
    }
    model->intercept = b_in;
    model->lambda = lambda;
    model->ratio = ratio;

    for (int iter = 0; iter < num_iters; iter++) {
        Vector *y_hat = vector_create(m);
        for (int i = 0; i < m; i++) {
            double dot = 0;
            for (int j = 0; j < n; j++) {
                dot += matrix_get(X, i, j) * vector_get(model->coef, j);
            }
            vector_set(y_hat, i, dot + model->intercept);
        }

        Vector *error = vector_arithmetic(y_hat, y, '-');

        if (model->fit_intercept == 1) {
            model->intercept = model->intercept - alpha * (vector_sum(error) / m);
        }

        for (int j = 0; j < n; j++) {
            double sum = 0;
            for (int i = 0; i < m; i++) {
                sum += vector_get(error, i) * matrix_get(X, i, j);
            }
            double w_j = vector_get(model->coef, j);

            switch (model->regularization_type) {
                case NO_REGULARIZATION: {
                    w_j = w_j - alpha * (sum/m);
                    break;
                }
                case L1_LASSO: {
                    w_j = w_j - alpha * (sum/m + sign(w_j)*lambda);
                    break;
                }
                case L2_RIDGE: {
                    w_j = w_j - alpha * (sum/m + 2*lambda*w_j);
                    break;
                }
                case ELASTIC_NET: {
                    const double penalty = lambda * (ratio * sign(w_j)+(1-ratio)*2*w_j);
                    w_j = w_j - alpha * (sum/m + penalty);
                    break;
                }
            }
            vector_set(model->coef, j, w_j);
        }

        if (print_iter == 1) {
            if (iter % 10 == 0) {
                double cost = 0;
                for (int e = 0; e < m; e++) {
                    cost += pow(vector_get(error, e), 2);
                }
                printf("Iteration: %d [%lf]\n", iter, cost / (2*m));
            }
        }

        vector_free(y_hat);
        vector_free(error);
    }
}

Vector *bgd_regression_predict(BGDRegression *model, Matrix *X) {
    if (!model) {
        NULL_ERROR("GDRegression model");
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
#include "linear_regression.h"

#include <stdlib.h>

LinearRegression *linear_regression_create(const int number_of_features, const int fit_intercept, const double l2_lambda) {
    if (number_of_features < 1) {
        INDEX_ERROR();
        return NULL;
    }
    if (fit_intercept != 0 && fit_intercept != 1) {
        CUSTOM_ERROR("Property 'fit_intercept' must be 0 or 1");
        return NULL;
    }

    LinearRegression *lr = malloc(sizeof(LinearRegression));
    lr->coef_ = vector_create(number_of_features);
    if (!lr->coef_) {
        ALLOCATION_ERROR();
        return NULL;
    }
    lr->intercept_ = 0;
    lr->number_of_features = number_of_features;
    lr->l2_lambda_ = l2_lambda;
    lr->fit_intercept = fit_intercept;

    return lr;
}

void linear_regression_free(LinearRegression *linear_regression) {
    if (!linear_regression) {
        NULL_LINEAR_REGRESSION_ERROR();
        return;
    }

    if (linear_regression->coef_) {
        vector_free(linear_regression->coef_);
    }
    free(linear_regression);
}

void linear_regression_fit(LinearRegression *model, Matrix *X, Vector *y) {
    if (!model) {
        NULL_LINEAR_REGRESSION_ERROR();
        return;
    }
    if (!X) {
        NULL_MATRIX_ERROR();
        return;
    }
    if (!y) {
        NULL_VECTOR_ERROR();
        return;
    }
    if (X->rows != y->dim || X->cols != model->number_of_features) {
        CUSTOM_ERROR("Dimension mismatch");
        return;
    }

    Matrix *X_use = NULL;
    if (model->fit_intercept == 0) {
        X_use = matrix_copy(X);
    } else {
        Matrix *X_1 = matrix_create(X->rows, 1);
        for (int i = 0; i < X->rows; i++) {
            matrix_set(X_1, i, 0, 1);
        }
        X_use = matrix_concat(X_1, X);
        matrix_free(X_1);
    }

    Matrix *lambda_I = matrix_create(X_use->cols, X_use->cols);
    if (!lambda_I) {
        NULL_MATRIX_ERROR();
        return;
    }
    for (int i = 0; i < lambda_I->cols; i++) {
        lambda_I->data[i * lambda_I->cols + i] = model->l2_lambda_;
    }

    if (model->fit_intercept == 1) {
        matrix_set(lambda_I, 0, 0, 0);
    }

    Matrix *y_mat = vector_to_matrix(y);
    Matrix *Xt = matrix_transpose(X_use, 0);
    Matrix *XtX = matrix_multiplication(Xt, X_use);
    Matrix *XtX_lambda = matrix_arithmetic(XtX, lambda_I, '+');
    Matrix *XtX_inv = matrix_inverse(XtX_lambda, 0);
    Matrix *XtX_inv_Xt = matrix_multiplication(XtX_inv, Xt);
    Matrix *w_mat = matrix_multiplication(XtX_inv_Xt, y_mat);

    if (model->fit_intercept == 0) {
        for (int i = 0; i < w_mat->rows; i++) {
            vector_set(model->coef_, i, matrix_get(w_mat, i, 0));
        }
    } else {
        model->intercept_ = matrix_get(w_mat, 0, 0);
        for (int i = 1; i < w_mat->rows; i++) {
            vector_set(model->coef_, i-1, matrix_get(w_mat, i, 0));
        }
    }

    matrix_free(X_use);
    matrix_free(lambda_I);
    matrix_free(y_mat);
    matrix_free(Xt);
    matrix_free(XtX);
    matrix_free(XtX_inv);
    matrix_free(XtX_inv_Xt);
    matrix_free(w_mat);
}

Vector *linear_regression_predict(LinearRegression *model, Matrix *X) {
    if (!model) {
        NULL_LINEAR_REGRESSION_ERROR();
        return NULL;
    }
    if (!X) {
        NULL_MATRIX_ERROR();
        return NULL;
    }

    Matrix *w_mat = vector_to_matrix(model->coef_);
    Matrix *y_hat = matrix_multiplication(X, w_mat);
    Vector *res = matrix_to_vector(y_hat, 0, 0, y_hat->rows);

    matrix_free(w_mat);
    matrix_free(y_hat);
    if (model->fit_intercept == 0) {
        return res;
    }

    for (int i = 0; i < res->dim; i++) {
        res->data[i] += model->intercept_;
    }
    return res;
}
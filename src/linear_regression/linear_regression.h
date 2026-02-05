#ifndef LINEAR_REGRESSION_H
#define LINEAR_REGRESSION_H

#include "../matrix/matrix.h"


typedef struct {
    Vector *coef_;
    double intercept_;
    double l2_lambda_;
    int fit_intercept;
    int number_of_features;
} LinearRegression;

LinearRegression *linear_regression_create(int number_of_features, int fit_intercept, double l2_lambda);
void linear_regression_free(LinearRegression *linear_regression);

void linear_regression_fit(LinearRegression *model, Matrix *X, Vector *y);
Vector *linear_regression_predict(LinearRegression *model, Matrix *X);

#endif
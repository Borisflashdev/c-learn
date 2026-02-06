#ifndef LINEAR_REGRESSION_H
#define LINEAR_REGRESSION_H

#include "../matrix/matrix.h"

typedef struct {
    Vector *coef;
    double intercept;
    double lambda;
    int fit_intercept;
    int number_of_features;
} LinearRegression;

LinearRegression *linear_regression_create(int number_of_features, int fit_intercept);
void linear_regression_free(LinearRegression *linear_regression);

void linear_regression_fit(LinearRegression *model, Matrix *X, Vector *y, double lambda);
Vector *linear_regression_predict(LinearRegression *model, Matrix *X);

#endif
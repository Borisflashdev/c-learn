#ifndef BGDREGRESSION_H
#define BGDREGRESSION_H

#include "../matrix/matrix.h"
#include "../math_functions/math_functions.h"

typedef enum {
    NO_REGULARIZATION,
    L1_LASSO,
    L2_RIDGE,
    ELASTIC_NET
} RegularizationType;

typedef struct {
    Vector *coef;
    double intercept;
    double lambda;
    double ratio;
    int fit_intercept;
    int number_of_features;
    RegularizationType regularization_type;
} BGDRegression;

BGDRegression *bgd_regression_create(int number_of_features, int fit_intercept, RegularizationType regularization_type);
void bgd_regression_free(BGDRegression *gd_regression);

void bgd_regression_fit(BGDRegression *model, Matrix *X, Vector *y, double w_in, double b_in, double alpha, int num_iters, double lambda, double ratio, int print_iter);
Vector *bgd_regression_predict(BGDRegression *model, Matrix *X);

#endif

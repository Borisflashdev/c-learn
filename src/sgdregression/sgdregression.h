#ifndef SGDREGRESSION_H
#define SGDREGRESSION_H

#include "../matrix/matrix.h"
#include "../math_functions/math_functions.h"
#include "../penalty_types/penalty_types.h"
#include "../random/random.h"

typedef struct {
    Vector *coef;
    double intercept;
    double lambda;
    double ratio;
    int fit_intercept;
    int number_of_features;
    int random_seed;
    Penalty penalty;
} SGDRegression;

SGDRegression *sgd_regression_create(int number_of_features, int fit_intercept, int random_seed, Penalty penalty);
void sgd_regression_free(SGDRegression *model);

void sgd_regression_fit(SGDRegression *model, Matrix *X, Vector *y, double alpha, int num_iters, double lambda, double ratio, int print_every);
Vector *sgd_regression_predict(SGDRegression *model, Matrix *X);

#endif

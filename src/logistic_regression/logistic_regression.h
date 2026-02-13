#ifndef LOGISTIC_REGRESSION_H
#define LOGISTIC_REGRESSION_H

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
    double threshold;
    Penalty penalty;
} LogisticRegression;

LogisticRegression *logistic_regression_create(int number_of_features, int fit_intercept, int random_seed, double threshold, Penalty penalty);
void logistic_regression_free(LogisticRegression *model);

void logistic_regression_fit(LogisticRegression *model, Matrix *X, Vector *y, int batch, double alpha, int num_iters, double lambda, double ratio, int print_every);
Vector *logistic_regression_predict_proba(LogisticRegression *model, Matrix *X);
Vector *logistic_regression_predict(LogisticRegression *model, Matrix *X);

#endif
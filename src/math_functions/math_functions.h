#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

typedef enum {
    NO_PENALTY,
    L1_LASSO,
    L2_RIDGE,
    ELASTIC_NET
} Penalty;

double math_sign(double num);
double math_xavier(double num);
double math_sigmoid(double num);

#endif
#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

double math_sign(double x);
double math_xavier(double fan_in, double fan_out);
double math_sigmoid(double x);
double math_relu(double x);
double math_leaky_relu(double x);
double math_silu(double x);
double math_tanh(double x);

double math_derivative_relu(double x);
double math_derivative_leaky_relu(double x);
double math_derivative_silu(double x);
double math_derivative_sigmoid(double x);
double math_derivative_tanh(double x);

#endif
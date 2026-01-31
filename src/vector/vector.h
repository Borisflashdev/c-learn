#ifndef VECTOR
#define VECTOR

#include "../errors/errors.h"

typedef struct {
    int dim;
    double *data;
} Vector;

Vector *vector_create(int dim);
Vector *vector_copy(const Vector *x);
void vector_free(Vector *x);

double vector_get(const Vector *x, int i);
void vector_set(const Vector *x, int i, double value);

void vector_print(const Vector *x);
void vector_print_head(const Vector *x, int num);
void vector_print_tail(const Vector *x, int num);

Vector *vector_arithmetic(const Vector *x, const Vector *y, char op);
void vector_scalar_arithmetic(const Vector *x, double scalar, char op);

double vector_min(const Vector *x);
double vector_max(const Vector *x);
double vector_sum(const Vector *x);
double vector_mean(const Vector *x);
double vector_std(const Vector *x, int ddof);

double vector_dot_product(const Vector *x, const Vector *y);
void vector_apply(const Vector *x, double (*func)(double));

#endif
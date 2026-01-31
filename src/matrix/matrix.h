#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>

#include "../errors/errors.h"
#include "../vector/vector.h"

typedef struct {
    int rows;
    int cols;
    double *data;
} Matrix;

Matrix *matrix_create(int rows, int cols);
Matrix *matrix_copy(const Matrix *X);
void matrix_free(Matrix *X);

double matrix_get(const Matrix *X, int i, int j);
void matrix_set(const Matrix *X, int i, int j, double value);

Matrix *read_csv(const char *path, char separator, int has_header);
void matrix_print(const Matrix *X);
void matrix_print_head(const Matrix *X, int num);
void matrix_print_tail(const Matrix *X, int num);

void matrix_shape(const Matrix *X);
double matrix_size(const Matrix *X);

Matrix *matrix_transpose(const Matrix *X);
Matrix *matrix_slice(const Matrix *X, int i_start, int i_end, int j_start, int j_end);
Matrix *matrix_slice_rows(const Matrix *X, int start, int end);
Matrix *matrix_slice_cols(const Matrix *X, int start, int end);
Matrix *matrix_concat(const Matrix *A, const Matrix *B);

Matrix *matrix_arithmetic(const Matrix *A, const Matrix *B, char op);
Matrix *matrix_multiplication(const Matrix *A, const Matrix *B);
void matrix_scalar_arithmetic(const Matrix *X, double scalar, char op);

double matrix_min(const Matrix *X);
double matrix_max(const Matrix *X);
double matrix_sum(const Matrix *X);
double matrix_mean(const Matrix *X);

double matrix_col_min(const Matrix *X, int col);
double matrix_col_max(const Matrix *X, int col);
double matrix_col_sum(const Matrix *X, int col);
double matrix_col_mean(const Matrix *X, int col);
double matrix_col_std(const Matrix *X, int col, int ddof);
double matrix_col_dot_product(const Matrix *A, int col_A, const Matrix *B, int col_B);
void matrix_apply_col(const Matrix *X, int col, double (*func)(double));

Matrix *vector_to_matrix(const Vector *x);
Vector *matrix_col_to_vector(const Matrix *X, int col);

#endif
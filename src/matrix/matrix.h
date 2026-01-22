#ifndef MATRIX_H
#define MATRIX_H
#include <stdio.h>

typedef struct {
    int rows;
    int cols;
    double *data;
} Matrix;

Matrix *matrix_create(int rows, int cols);
void matrix_free(Matrix *X);

double matrix_get(const Matrix *X, int i, int j);
void matrix_set(Matrix *X, int i, int j, double value);

Matrix *read_csv(const char *path, char separator, int has_header);
void matrix_print(const Matrix *X);

Matrix *slice_rows(const Matrix *X, int start, int end);
Matrix *slice_cols(const Matrix *X, int start, int end);

void shape(const Matrix *X);
double size(const Matrix *X);

Matrix *matrix_transpose(const Matrix *X);

Matrix *matrix_arithmetic(Matrix *A, Matrix *B, char op);
Matrix *matrix_multiplication(Matrix *A, Matrix *B);

void matrix_scalar_arithmetic(Matrix *X, double scalar, char op);

Matrix *matrix_concat(Matrix *A, Matrix *B);

Matrix *matrix_copy(Matrix *X);

double matrix_sum(const Matrix *X);
double matrix_mean(const Matrix *X);

double matrix_col_sum(const Matrix *X, int col);
double matrix_col_mean(const Matrix *X, int col);
double matrix_col_std(const Matrix *X, int col, int ddof);

#endif
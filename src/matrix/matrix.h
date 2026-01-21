#ifndef MATRIX_H
#define MATRIX_H
#include <stdio.h>

typedef struct {
    int rows;
    int cols;
    double *data;
} Matrix;

Matrix create_matrix(int rows, int cols);
void free_matrix(Matrix *X);

double get_matrix_element(const Matrix *X, int i, int j);
void set_matrix_element(Matrix *X, int i, int j, double value);

Matrix *read_csv(const char *path, char separator, int has_header);
void print_matrix(const Matrix *X);

Matrix *slice_rows(const Matrix *X, int start, int end);
Matrix *slice_cols(const Matrix *X, int start, int end);
Matrix *slice_matrix(const Matrix *X, int start_i, int end_i, int start_j, int end_j);

void shape(const Matrix *X);
void size(const Matrix *X);

Matrix *matrix_transpose(const Matrix *X);

Matrix *matrix_arithmetic(Matrix *A, Matrix *B, char op);
Matrix *matrix_multiplication(Matrix *A, Matrix *B);

Matrix *matrix_scalar_arithmetic(Matrix *X, double scalar, char op);

Matrix *matrix_concat(Matrix *A, Matrix *B);

#endif
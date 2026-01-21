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

double get(const Matrix *X, int i, int j);
void set(Matrix *X, int i, int j, double value);

Matrix *read_csv(const char *path, char separator, int has_header);
void print_matrix(const Matrix *X);

Matrix *slice_rows(const Matrix *X, int start, int end);
Matrix *slice_cols(const Matrix *X, int start, int end);
Matrix *slice(const Matrix *X, int start_i, int end_i, int start_j, int end_j);

#endif
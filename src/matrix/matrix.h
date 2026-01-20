#ifndef MATRIX_H
#define MATRIX_H
#include <stdio.h>

typedef struct {
    int rows;
    int cols;
    double *data;
} Matrix;

Matrix create_matrix(int rows, int cols);
void free_matrix(Matrix *matrix);

double get(Matrix *X, int i, int j);
void set(Matrix *X, int i, int j, double value);

Matrix *read_csv(const char *path, char separator, int has_header);

#endif
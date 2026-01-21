#include "matrix.h"
#include <stdlib.h>
#include <string.h>

Matrix create_matrix(const int rows, const int cols) {
    Matrix matrix;
    matrix.rows = rows;
    matrix.cols = cols;
    matrix.data = malloc(rows * cols * sizeof(double));
    return matrix;
}

void free_matrix(Matrix *X) {
    free(X->data);
    free(X);
}

double get(const Matrix *X, const int i, const int j) {
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        printf("Error: Index Out of Bounds\n");
        return 0;
    }
    return X->data[i * X->cols + j];
}

void set(Matrix *X, const int i, const int j, const double value) {
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        printf("Error: Index Out of Bounds\n");
        return;
    }
    X->data[i * X->cols + j] = value;
}

Matrix *read_csv(const char *path, const char separator, const int has_header) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("Error: File %s not found\n", path);
        return NULL;
    }

    char line[1024];
    //------------------------BOM------------------------//
    int is_bom = 0;
    const int c1 = fgetc(file);
    const int c2 = fgetc(file);
    const int c3 = fgetc(file);
    if (c1 == 0xEF && c2 == 0xBB && c3 == 0xBF) {
        is_bom = 1;
    }
    //--------------------Count cols--------------------//
    fgets(line, sizeof(line), file);
    int cols = 1;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == separator) {
            cols++;
        }
    }
    //--------------------Count rows--------------------//
    int rows = 1;
    while(fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) {
            rows++;
        }
    }
    if (has_header == 1) {
        rows--;
    }
    if (cols == 1) {
        rows++;
    }
    //--------------------------------------------------//

    fseek(file, 0, SEEK_SET);

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(rows, cols);

    if (has_header == 1) {
        fgets(line, sizeof(line), file);
    }

    int i=0;
    while(fgets(line, sizeof(line), file)) {
        int j=0;
        const char sep[2] = {separator, '\0'};
        const char *token = strtok(line, sep);
        if (i==0 && is_bom==1 && has_header == 0) {
            memmove(token, token + 3, strlen(token + 3) + 1);
        }
        while(token) {
            const double value = atof(token);
            set(matrix, i, j, value);
            token = strtok(NULL, sep);
            j++;
        }
        i++;
    }

    fclose(file);
    return matrix;
}

void print_matrix(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return;
    }
    printf("[");
    for (int i = 0; i < X->rows; i++) {
        if (i > 0) printf("\n ");
        printf("[");
        for (int j = 0; j < X->cols; j++) {
            printf("%lf", get(X, i, j));
            if (j < X->cols - 1) printf(", ");
        }
        printf("]");
    }
    printf("]\n");
}

Matrix *slice_rows(const Matrix *X, const int start, const int end) {
    if (!X || start < 0 || end < 0 || start >= X->rows || end > X->rows || start >= end) {
        printf("Error: Index Out of Bounds\n");
        return NULL;
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(end-start, X->cols);

    int n_i = 0;
    for (int i = start; i < end; i++) {
        for (int j = 0; j < X->cols; j++) {
            set(matrix, n_i, j, get(X, i, j));
        }
        n_i++;
    }
    return matrix;
}

Matrix *slice_cols(const Matrix *X, const int start, const int end) {
    if (!X || start < 0 || end < 0 || start >= X->cols || end > X->cols || start >= end) {
        printf("Error: Index Out of Bounds\n");
        return NULL;
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(X->rows, end-start);

    int n_j = 0;
    for (int i = 0; i < X->rows; i++) {
        for (int j = start; j < end; j++) {
            set(matrix, i, n_j, get(X, i, j));
            n_j++;
        }
        n_j = 0;
    }
    return matrix;
}

Matrix *slice(const Matrix *X, int start_i, int end_i, int start_j, int end_j) {
    if (!X || start_i < 0 || end_i < 0 || start_j < 0 || end_j < 0 || start_i >= end_i || start_j >= end_j || start_i >= X->rows || end_i > X->rows || start_j >= X->cols || end_j > X->cols)
    {
        printf("Error: Index Out of Bounds\n");
        return NULL;
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(end_i-start_i, end_j-start_j);

    int n_i = 0;
    int n_j = 0;
    for (int i = start_i; i < end_i; i++) {
        for (int j = start_j; j < end_j; j++) {
            set(matrix, n_i, n_j, get(X, i, j));
            n_j++;
        }
        n_i++;
        n_j = 0;
    }

    return matrix;
}

void shape(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return;
    }

    printf("(%d, %d)", X->rows, X->cols);
}

void size(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return;
    }

    printf("%d", X->rows*X->cols);
}

Matrix *matrix_transpose(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return NULL;
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(X->cols, X->rows);

    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            set(matrix, j, i, get(X, i, j));
        }
    }

    return matrix;
}

Matrix *matrix_arithmetic(Matrix *A, Matrix *B, const char op) {
    if (A == NULL) {
        printf("Error: Matrix A is NULL\n");
        return NULL;
    }
    if (B == NULL) {
        printf("Error: Matrix B is NULL\n");
        return NULL;
    }
    if (A->cols != B->cols || A->rows != B->rows) {
        printf("Error: Matrix dimensions are not same\n");
        return NULL;
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(A->cols, A->rows);

    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            if (op == '+') {
                set(matrix, i, j, get(A, i, j)+get(B, i, j));
            } else if (op == '-') {
                set(matrix, i, j, get(A, i, j)-get(B, i, j));
            } else if (op == '*') {
                set(matrix, i, j, get(A, i, j)*get(B, i, j));
            } else if (op == '/') {
                set(matrix, i, j, get(A, i, j)/get(B, i, j));
            } else {
                printf("Error: Invalid Operation\n");
                free_matrix(matrix);
                return NULL;
            }
        }
    }

    return matrix;
}

Matrix *matrix_multiplication(Matrix *A, Matrix *B) {
    if (A == NULL) {
        printf("Error: Matrix A is NULL\n");
        return NULL;
    }
    if (B == NULL) {
        printf("Error: Matrix B is NULL\n");
        return NULL;
    }
    if (A->cols != B->rows) {
        printf("Error: Matrix dimensions are not defined\n");
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(A->rows, B->cols);

    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < A->cols; k++) {
                sum += get(A, i, k) * get(B, k, j);
            }
            set(matrix, i, j, sum);
        }
    }

    return matrix;
}

Matrix *matrix_scalar_arithmetic(Matrix *X, double scalar, char op) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return NULL;
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(X->rows, X->cols);

    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            if (op == '+') {
                set(matrix, i, j, get(X, i, j)+scalar);
            } else if (op == '-') {
                set(matrix, i, j, get(X, i, j)-scalar);
            } else if (op == '*') {
                set(matrix, i, j, get(X, i, j)*scalar);
            } else if (op == '/') {
                set(matrix, i, j, get(X, i, j)/scalar);
            } else {
                printf("Error: Invalid Operation\n");
                free_matrix(matrix);
                return NULL;
            }
        }
    }

    return matrix;
}

Matrix *matrix_concat(Matrix *A, Matrix *B) {
    if (A == NULL) {
        printf("Error: Matrix A is NULL\n");
    }
    if (B == NULL) {
        printf("Error: Matrix B is NULL\n");
        return NULL;
    }
    if (A->rows != B->rows) {
        printf("Error: Matrix rows are not same\n");
        return NULL;
    }

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(A->rows, A->cols+B->cols);

    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            set(matrix, i, j, get(A, i, j));
        }
    }

    for (int i = 0; i < B->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            set(matrix, i, j + A->cols, get(B, i, j));
        }
    }

    return matrix;
}
#include "matrix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

Matrix *matrix_create(const int rows, const int cols) {
    Matrix *X = malloc(sizeof(Matrix));
    if (X == NULL) return NULL;

    X->rows = rows;
    X->cols = cols;
    X->data = calloc(rows * cols, sizeof(double));

    return X;
}

void matrix_free(Matrix *X) {
    if (X) {
        free(X->data);
        free(X);
    }
}

double matrix_get(const Matrix *X, const int i, const int j) {
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        printf("Error: Index Out of Bounds\n");
        return 0;
    }
    return X->data[i * X->cols + j];
}

void matrix_set(Matrix *X, const int i, const int j, const double value) {
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

    Matrix *matrix = matrix_create(rows, cols);

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
           matrix_set(matrix, i, j, value);
            token = strtok(NULL, sep);
            j++;
        }
        i++;
    }

    fclose(file);
    return matrix;
}

void matrix_print(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return;
    }
    printf("[");
    for (int i = 0; i < X->rows; i++) {
        if (i > 0) printf("\n ");
        printf("[");
        for (int j = 0; j < X->cols; j++) {
            printf("%lf",matrix_get(X, i, j));
            if (j < X->cols - 1) printf(", ");
        }
        printf("]");
    }
    printf("]\n");
}

void matrix_print_head(const Matrix *X, int num) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return;
    }
    if (num > X->rows || num < 1) {
        printf("Error: Index Out of Bounds\n");
        return;
    }

    printf("[");
    for (int i = 0; i < num; i++) {
        if (i > 0) printf("\n ");
        printf("[");
        for (int j = 0; j < X->cols; j++) {
            printf("%lf",matrix_get(X, i, j));
            if (j < X->cols - 1) printf(", ");
        }
        printf("]");
    }
    if (num == X->rows) {
        printf("]\n");
    } else {
        printf("...]\n");
    }
}

void matrix_print_tail(const Matrix *X, int num) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return;
    }
    if (num > X->rows || num < 1) {
        printf("Error: Index Out of Bounds\n");
        return;
    }

    if (num == X->rows) {
        printf("[");
    } else {
        printf("[...");
    }
    for (int i = X->rows - num; i < X->rows; i++) {
        if (i > X->rows - num) printf("\n ");
        printf("[");
        for (int j = 0; j < X->cols; j++) {
            printf("%lf", matrix_get(X, i, j));
            if (j < X->cols - 1) printf(", ");
        }
        printf("]");
    }
    printf("]\n");
}

Matrix *matrix_slice_rows(const Matrix *X, const int start, const int end) {
    if (!X || start < 0 || end < 0 || start >= X->rows || end > X->rows || start >= end) {
        printf("Error: Index Out of Bounds\n");
        return NULL;
    }

    Matrix* matrix = matrix_create(end-start, X->cols);

    int n_i = 0;
    for (int i = start; i < end; i++) {
        for (int j = 0; j < X->cols; j++) {
           matrix_set(matrix, n_i, j,matrix_get(X, i, j));
        }
        n_i++;
    }
    return matrix;
}

Matrix *matrix_slice_cols(const Matrix *X, const int start, const int end) {
    if (!X || start < 0 || end < 0 || start >= X->cols || end > X->cols || start >= end) {
        printf("Error: Index Out of Bounds\n");
        return NULL;
    }

    Matrix* matrix = matrix_create(X->rows, end-start);

    int n_j = 0;
    for (int i = 0; i < X->rows; i++) {
        for (int j = start; j < end; j++) {
           matrix_set(matrix, i, n_j,matrix_get(X, i, j));
            n_j++;
        }
        n_j = 0;
    }
    return matrix;
}

void matrix_shape(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return;
    }

    printf("(%d, %d)", X->rows, X->cols);
}

double matrix_size(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return 0;
    }

    return X->rows*X->cols;
}

Matrix *matrix_transpose(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix is NULL\n");
        return NULL;
    }

    Matrix* matrix = matrix_create(X->cols, X->rows);

    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
           matrix_set(matrix, j, i,matrix_get(X, i, j));
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

    Matrix* matrix = matrix_create(A->rows, A->cols);

    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            if (op == '+') {
               matrix_set(matrix, i, j,matrix_get(A, i, j)+matrix_get(B, i, j));
            } else if (op == '-') {
               matrix_set(matrix, i, j,matrix_get(A, i, j)-matrix_get(B, i, j));
            } else if (op == '*') {
               matrix_set(matrix, i, j,matrix_get(A, i, j)*matrix_get(B, i, j));
            } else if (op == '/') {
               matrix_set(matrix, i, j,matrix_get(A, i, j)/matrix_get(B, i, j));
            } else {
                printf("Error: Invalid Operation\n");
                matrix_free(matrix);
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
        printf("Error: Matrix dimensions are incompatible for multiplication\n");
        return NULL;
    }

    Matrix* matrix = matrix_create(A->rows, B->cols);

    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < A->cols; k++) {
                sum +=matrix_get(A, i, k) *matrix_get(B, k, j);
            }
           matrix_set(matrix, i, j, sum);
        }
    }

    return matrix;
}

void matrix_scalar_arithmetic(Matrix *X, double scalar, char op) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return;
    }

    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            if (op == '+') {
               matrix_set(X, i, j,matrix_get(X, i, j)+scalar);
            } else if (op == '-') {
               matrix_set(X, i, j,matrix_get(X, i, j)-scalar);
            } else if (op == '*') {
               matrix_set(X, i, j,matrix_get(X, i, j)*scalar);
            } else if (op == '/') {
               matrix_set(X, i, j,matrix_get(X, i, j)/scalar);
            } else {
                printf("Error: Invalid Operation\n");
                return ;
            }
        }
    }
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

    Matrix* matrix = matrix_create(A->rows, A->cols+B->cols);

    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
           matrix_set(matrix, i, j,matrix_get(A, i, j));
        }
    }

    for (int i = 0; i < B->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
           matrix_set(matrix, i, j + A->cols,matrix_get(B, i, j));
        }
    }

    return matrix;
}

Matrix *matrix_copy(Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return NULL;
    }

    Matrix* matrix = matrix_create(X->rows, X->cols);
    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            matrix_set(matrix, i, j, matrix_get(X, i, j));
        }
    }

    return matrix;
}

double matrix_sum(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return 0;
    }

    double sum=0;
    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            sum += matrix_get(X, i, j);
        }
    }

    return sum;
}

double matrix_mean(const Matrix *X) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return 0;
    }

    double sum = 0;
    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            sum += matrix_get(X, i, j);
        }
    }

    return sum / matrix_size(X);
}

double matrix_col_sum(const Matrix *X, int col) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        printf("Error: Error: Index Out of Bounds\n");
        return 0;
    }

    double sum = 0.0;
    for (int i = 0; i < X->rows; i++) {
        sum += matrix_get(X, i, col);
    }

    return sum;
}

double matrix_col_mean(const Matrix *X, int col) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        printf("Error: Index Out of Bounds\n");
        return 0;
    }

    return matrix_col_sum(X, col) / X->rows;
}

double matrix_col_std(const Matrix *X, int col, int ddof) {
    if (X == NULL) {
        printf("Error: Matrix X is NULL\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        printf("Error: Index Out of Bounds\n");
        return 0;
    }

    double mean = matrix_col_mean(X, col);
    double var = 0.0;

    for (int i = 0; i < X->rows; i++) {
        double diff = matrix_get(X, i, col) - mean;
        var += diff * diff;
    }

    if (ddof==0) {
        var /= X->rows;
    } else {
        var /= X->rows-1;
    }
    return sqrt(var);
}

double dot_product(const Matrix *A, const Matrix *B) {
    if (A == NULL) {
        printf("Error: Matrix A is NULL\n");
        return 0;
    }
    if (B == NULL) {
        printf("Error: Matrix B is NULL\n");
        return 0;
    }
    if (A->rows != B->rows ) {
        printf("Error: Vector dimensions are not the same\n");
        return 0;
    }
    if (A->cols != 1 || B->cols != 1) {
        printf("Error: Dot product is defined only for vectors\n");
        return 0;
    }

    double sum = 0.0;
    for (int i = 0; i < A->rows; i++) {
        sum += matrix_get(A, i, 0) * matrix_get(B, i, 0);
    }
    return sum;
}
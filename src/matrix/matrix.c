#include "matrix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

Matrix *matrix_create(const int rows, const int cols) {
    if (rows < 1 || cols < 1) {
        fprintf(stderr, "Error: Invalid matrix dimensions\n");
        return NULL;
    }
    Matrix *X = malloc(sizeof(Matrix));
    if (X == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    X->rows = rows;
    X->cols = cols;
    X->data = calloc(rows * cols, sizeof(double));

    return X;
}

Matrix *matrix_copy(const Matrix *X) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot copy NULL matrix\n");
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

void matrix_free(Matrix *X) {
    if (X) {
        free(X->data);
        free(X);
    } else {
        fprintf(stderr, "Error: Cannot free NULL matrix\n");
    }
}

double matrix_get(const Matrix *X, const int i, const int j) {
    if (!X) {
        fprintf(stderr, "Error: Cannot get element in NULL matrix\n");
        return 0;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return 0;
    }
    return X->data[i * X->cols + j];
}

void matrix_set(const Matrix *X, const int i, const int j, const double value) {
    if (!X) {
        fprintf(stderr, "Error: Cannot set element in NULL matrix\n");
        return;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return;
    }
    X->data[i * X->cols + j] = value;
}

Matrix *read_csv(const char *path, const char separator, const int has_header) {
    if (has_header < 0 || has_header > 1) {
        fprintf(stderr, "Error: Property 'has_header' must be 0 or 1\n");
        return NULL;
    }
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr,"Error: File %s not found\n", path);
        return NULL;
    }

    char line[1024];

    // BOM
    const int c1 = fgetc(file);
    const int c2 = fgetc(file);
    const int c3 = fgetc(file);
    if (!(c1 == 0xEF && c2 == 0xBB && c3 == 0xBF)) {
        rewind(file);
    }

    if (has_header == 1) {
        fgets(line, sizeof(line), file);
    }

    int rows = 0;
    int cols = 0;
    const long data_start = ftell(file);

    while(fgets(line, sizeof(line), file)) {
        if (rows == 0) {
            cols = 1;
            for (int i = 0; line[i] != '\0' && line[i] != '\n' && line[i] != '\r'; i++) {
                if (line[i] == separator) {
                    cols++;
                }
            }
        }
        rows++;
    }

    if (rows == 0 || cols == 0) {
        fprintf(stderr, "Error: Empty CSV file\n");
        fclose(file);
        return NULL;
    }

    Matrix *matrix = matrix_create(rows, cols);
    if (!matrix) {
        fclose(file);
        return NULL;
    }

    fseek(file, data_start, SEEK_SET);

    int i = 0;
    while(fgets(line, sizeof(line), file) && i < rows) {
        int j = 0;
        const char sep[2] = {separator, '\0'};
        char *token = strtok(line, sep);
        while(token && j < cols) {
            matrix_set(matrix, i, j, atof(token));
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
        fprintf(stderr, "Error: Cannot print NULL matrix\n");
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

void matrix_print_head(const Matrix *X, const int num) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot print NULL matrix\n");
        return;
    }
    if (num > X->rows || num < 1) {
        fprintf(stderr, "Error: Index out of bounds\n");
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

void matrix_print_tail(const Matrix *X, const int num) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot print NULL matrix\n");
        return;
    }
    if (num > X->rows || num < 1) {
        fprintf(stderr, "Error: Index out of bounds\n");
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

void matrix_shape(const Matrix *X) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot get shape of NULL matrix\n");
        return;
    }

    printf("(%d, %d)", X->rows, X->cols);
}

double matrix_size(const Matrix *X) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot get size of NULL matrix\n");
        return 0;
    }

    return X->rows*X->cols;
}

Matrix *matrix_transpose(const Matrix *X) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot transpose NULL matrix\n");
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

Matrix *matrix_slice_rows(const Matrix *X, const int start, const int end) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot slice NULL matrix\n");
        return NULL;
    }
    if (start < 0 || end < 0 || start >= X->rows || end > X->rows || start >= end) {
        fprintf(stderr, "\nError: Index out of bounds\n");
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
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot slice NULL matrix\n");
        return NULL;
    }
    if (start < 0 || end < 0 || start >= X->cols || end > X->cols || start >= end) {
        fprintf(stderr, "\nError: Index out of bounds\n");
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

Matrix *matrix_concat(const Matrix *A, const Matrix *B) {
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Error: Cannot concatenate NULL matrix\n");
        return NULL;
    }
    if (A->rows != B->rows) {
        fprintf(stderr, "Error: Matrix row dimensions must match\n");
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

Matrix *matrix_arithmetic(const Matrix *A, const Matrix *B, const char op) {
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Error: Cannot perform arithmetic on NULL matrix.\n");
        return NULL;
    }
    if (A->cols != B->cols || A->rows != B->rows) {
        fprintf(stderr, "Error: Matrix dimensions must match\n");
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
                fprintf(stderr, "Error: Invalid operator\n");
                matrix_free(matrix);
                return NULL;
            }
        }
    }

    return matrix;
}

Matrix *matrix_multiplication(const Matrix *A, const Matrix *B) {
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Error: Cannot multiply NULL matrix\n");
        return NULL;
    }
    if (A->cols != B->rows) {
        fprintf(stderr, "Error: Incompatible dimensions for multiplication\n");
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

void matrix_scalar_arithmetic(const Matrix *X, const double scalar, const char op) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot perform scalar arithmetic on NULL matrix.\n");
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
               if (scalar == 0.0) {
                   fprintf(stderr, "Error: Division by zero is not allowed.\n");
                   return;
               }
               matrix_set(X, i, j,matrix_get(X, i, j)/scalar);
            } else {
                fprintf(stderr, "Error: Invalid operator\n");
                return;
            }
        }
    }
}

double matrix_sum(const Matrix *X) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot calculate sum of NULL matrix.\n");
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
        fprintf(stderr, "Error: Cannot calculate mean of NULL matrix.\n");
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

double matrix_col_sum(const Matrix *X, const int col) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot calculate column sum of NULL matrix.\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return 0;
    }

    double sum = 0.0;
    for (int i = 0; i < X->rows; i++) {
        sum += matrix_get(X, i, col);
    }

    return sum;
}

double matrix_col_mean(const Matrix *X, const int col) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot calculate column mean of NULL matrix.\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return 0;
    }

    return matrix_col_sum(X, col) / X->rows;
}

double matrix_col_std(const Matrix *X, const int col, const int ddof) {
    if (ddof < 0 || ddof > 1) {
        fprintf(stderr, "\nError: Property 'ddof' must be 0 or 1\n");
    }
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot calculate standard deviation of NULL matrix.\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return 0;
    }

    const double mean = matrix_col_mean(X, col);
    double var = 0.0;

    for (int i = 0; i < X->rows; i++) {
        const double diff = matrix_get(X, i, col) - mean;
        var += diff * diff;
    }

    if (ddof==0) {
        var /= X->rows;
    } else {
        var /= X->rows-1;
    }
    return sqrt(var);
}

double matrix_col_dot_product(const Matrix *A, const int col_A, const Matrix *B, const int col_B) {
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Error: Cannot compute dot product with NULL matrix.\n");
        return 0.0;
    }
    if (A->rows != B->rows) {
        fprintf(stderr, "Error: Row dimensions must match for dot product\n");
        return 0.0;
    }
    if (col_A < 0 || col_A >= A->cols) {
        fprintf(stderr, "Error: Index col_A out of bounds\n");
        return 0;
    }
    if (col_B < 0 || col_B >= B->cols) {
        fprintf(stderr, "Error: Index col_B out of bounds\n");
        return 0;
    }


    double sum = 0.0;
    for (int i = 0; i < A->rows; i++) {
        sum += matrix_get(A, i, col_A) * matrix_get(B, i, col_B);
    }
    return sum;
}

void matrix_apply_col(const Matrix *X, const int col, double (*func)(double)) {
    if (X == NULL) {
        fprintf(stderr, "Error: Cannot apply function to NULL matrix.\n");
        return;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "Error: Index out of bounds\n");
        return;
    }
    if (func == NULL) {
        fprintf(stderr, "Error: Function pointer is NULL.\n");
        return;
    }

    for (int i = 0; i < X->rows; i++) {
        const double val = matrix_get(X, i, col);
        matrix_set(X, i, col, func(val));
    }
}
#include "matrix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

Matrix *matrix_create(const int rows, const int cols) {
    if (rows < 1 || cols < 1) {
        fprintf(stderr, "\n\nError: Invalid matrix dimensions\n");
        return NULL;
    }
    Matrix *X = malloc(sizeof(Matrix));
    if (!X) {
        fprintf(stderr, "\n\nError: Memory allocation failed\n");
        return NULL;
    }

    X->rows = rows;
    X->cols = cols;
    X->data = calloc(rows * cols, sizeof(double));

    return X;
}

Matrix *matrix_copy(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\n\nError: Cannot copy NULL matrix\n");
        return NULL;
    }

    Matrix* copy = matrix_create(X->rows, X->cols);
    if (!copy) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return NULL;
    }
    memcpy(copy->data, X->data, sizeof(double) * X->rows * X->cols);

    return copy;
}

void matrix_free(Matrix *X) {
    if (X) {
        free(X->data);
        free(X);
    } else {
        fprintf(stderr, "\nError: Cannot free NULL matrix\n");
    }
}

double matrix_get(const Matrix *X, const int i, const int j) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot get element in NULL matrix\n");
        return 0;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return 0;
    }
    return X->data[i * X->cols + j];
}

void matrix_set(const Matrix *X, const int i, const int j, const double value) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot set element in NULL matrix\n");
        return;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return;
    }
    X->data[i * X->cols + j] = value;
}

Matrix *read_csv(const char *path, const char separator, const int has_header) {
    if (has_header < 0 || has_header > 1) {
        fprintf(stderr, "\nError: Property 'has_header' must be 0 or 1\n");
        return NULL;
    }
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr,"\nError: File %s not found\n", path);
        return NULL;
    }

    char line[1024];

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
        fprintf(stderr, "\nError: Empty CSV file\n");
        fclose(file);
        return NULL;
    }

    Matrix *X = matrix_create(rows, cols);
    if (!X) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    fseek(file, data_start, SEEK_SET);

    int i = 0;
    while(fgets(line, sizeof(line), file) && i < rows) {
        int j = 0;
        const char sep[2] = {separator, '\0'};
        const char *token = strtok(line, sep);
        while(token && j < cols) {
            matrix_set(X, i, j, atof(token));
            token = strtok(NULL, sep);
            j++;
        }
        i++;
    }

    fclose(file);
    return X;
}

void matrix_print(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot print NULL matrix\n");
        return;
    }

    const size_t buf_size = X->rows * X->cols * 32 + 64;
    char *buf = malloc(buf_size);
    if (!buf) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return;
    }

    char *p = buf;
    p += sprintf(p, "[");
    for (int i = 0; i < X->rows; i++) {
        if (i > 0) p += sprintf(p, "\n ");
        p += sprintf(p, "[");

        for (int j = 0; j < X->cols; j++) {
            p += sprintf(p, "%.6f", X->data[i * X->cols + j]);
            if (j < X->cols - 1) p += sprintf(p, ", ");
        }
        p += sprintf(p, "]");
    }
    p += sprintf(p, "]\n");

    fwrite(buf, 1, p - buf, stdout);
    free(buf);
}

void matrix_print_head(const Matrix *X, const int num) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot print NULL matrix\n");
        return;
    }
    if (num < 1 || num > X->rows) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return;
    }

    printf("[");
    for (int i = 0; i < num; i++) {
        if (i > 0) printf("\n ");
        printf("[");
        for (int j = 0; j < X->cols; j++) {
            printf("%.6f", X->data[i * X->cols + j]);
            if (j < X->cols - 1) printf(", ");
        }
        printf("]");
    }
    printf(num == X->rows ? "]\n" : "...]\n");
}

void matrix_print_tail(const Matrix *X, const int num) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot print NULL matrix\n");
        return;
    }
    if (num > X->rows || num < 1) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return;
    }

    const int start = X->rows - num;

    printf(num == X->rows ? "[" : "[...");
    for (int i = start; i < X->rows; i++) {
        if (i > start) printf("\n ");
        printf("[");
        for (int j = 0; j < X->cols; j++) {
            printf("%.6f", X->data[i * X->cols + j]);
            if (j < X->cols - 1) printf(", ");
        }
        printf("]");
    }
    printf("]\n");
}

void matrix_shape(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot get shape of NULL matrix\n");
        return;
    }

    printf("(%d, %d)", X->rows, X->cols);
}

double matrix_size(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot get size of NULL matrix\n");
        return 0;
    }

    return X->rows*X->cols;
}

Matrix *matrix_transpose(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot transpose NULL matrix\n");
        return NULL;
    }

    Matrix* transposed_matrix = matrix_create(X->cols, X->rows);
    if (!transposed_matrix) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            transposed_matrix->data[j * transposed_matrix->cols + i] =X->data[i * X->cols + j];
        }
    }

    return transposed_matrix;
}

Matrix *matrix_slice_rows(const Matrix *X, const int start, const int end) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot slice NULL matrix\n");
        return NULL;
    }
    if (start < 0 || end < 0 || start >= X->rows || end > X->rows || start >= end) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return NULL;
    }

    const int rows = end - start;
    Matrix* slice = matrix_create(rows, X->cols);
    if (!slice) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        memcpy(slice->data + i * X->cols, X->data + (start + i) * X->cols, sizeof(double) * X->cols);
    }
    return slice;
}

Matrix *matrix_slice_cols(const Matrix *X, const int start, const int end) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot slice NULL matrix\n");
        return NULL;
    }
    if (start < 0 || end < 0 || start >= X->cols || end > X->cols || start >= end) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return NULL;
    }

    const int cols = end - start;
    Matrix* slice = matrix_create(X->rows, cols);
    if (!slice) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < cols; j++) {
            slice->data[i * cols + j] = X->data[i * X->cols + start + j];
        }
    }
    return slice;
}

Matrix *matrix_concat(const Matrix *A, const Matrix *B) {
    if (!A || !B) {
        fprintf(stderr, "\nError: Cannot concatenate NULL matrix\n");
        return NULL;
    }
    if (A->rows != B->rows) {
        fprintf(stderr, "\nError: Matrix row dimensions must match\n");
        return NULL;
    }

    const int cols = A->cols + B->cols;
    Matrix* C = matrix_create(A->rows, cols);
    if (!C) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < A->rows; i++) {
        memcpy(C->data + i * cols, A->data + i * A->cols, sizeof(double) * A->cols);
        memcpy(C->data + i * cols + A->cols, B->data + i * B->cols, sizeof(double) * B->cols);
    }

    return C;
}

Matrix *matrix_arithmetic(const Matrix *A, const Matrix *B, const char op) {
    if (!A || !B) {
        fprintf(stderr, "\nError: Cannot perform arithmetic on NULL matrix\n");
        return NULL;
    }
    if (A->cols != B->cols || A->rows != B->rows) {
        fprintf(stderr, "\nError: Matrix dimensions must match\n");
        return NULL;
    }

    Matrix* C = matrix_create(A->rows, A->cols);
    if (!C) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return NULL;
    }

    const int size = A->rows * A->cols;
    switch (op) {
        case '+':
            for (int i = 0; i < size; i++)
                C->data[i] = A->data[i] + B->data[i];
            break;
        case '-':
            for (int i = 0; i < size; i++)
                C->data[i] = A->data[i] - B->data[i];
            break;
        case '*':
            for (int i = 0; i < size; i++)
                C->data[i] = A->data[i] * B->data[i];
            break;
        case '/':
            for (int i = 0; i < size; i++) {
                if (B->data[i] == 0) {
                    fprintf(stderr, "\nError: Division by zero\n");
                    matrix_free(C);
                    return NULL;
                }
                C->data[i] = A->data[i] / B->data[i];
            }
            break;
        default:
            fprintf(stderr, "\nError: Invalid operator '%c'\n", op);
            matrix_free(C);
            return NULL;
    }
    return C;
}

// TODO: try faster multiply (strassen? winograd)
Matrix *matrix_multiplication(const Matrix *A, const Matrix *B) {
    if (!A || !B) {
        fprintf(stderr, "\nError: Cannot multiply NULL matrix\n");
        return NULL;
    }
    if (A->cols != B->rows) {
        fprintf(stderr, "\nError: Incompatible dimensions for multiplication\n");
        return NULL;
    }

    Matrix* C = matrix_create(A->rows, B->cols);
    if (!C) {
        fprintf(stderr, "\nError: Memory allocation failed\n");
        return NULL;
    }

    const int m = A->rows;
    const int n = A->cols;
    const int p = B->cols;

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            double sum = 0;
            for (int k = 0; k < n; k++) {
                sum += A->data[i * n + k] * B->data[k * p + j];
            }
            C->data[i * p + j] = sum;
        }
    }

    return C;
}

void matrix_scalar_arithmetic(const Matrix *X, const double scalar, const char op) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot perform scalar arithmetic on NULL matrix\n");
        return;
    }

    const int size = X->rows * X->cols;
    switch (op) {
        case '+':
            for (int i = 0; i < size; i++)
                X->data[i] += scalar;
            break;
        case '-':
            for (int i = 0; i < size; i++)
                X->data[i] -= scalar;
            break;
        case '*':
            for (int i = 0; i < size; i++)
                X->data[i] *= scalar;
            break;
        case '/':
            if (scalar == 0) {
                fprintf(stderr, "\nError: Division by zero\n");
                return;
            }
            for (int i = 0; i < size; i++)
                X->data[i] /= scalar;
            break;
        default:
            fprintf(stderr, "\nError: Invalid operator '%c'\n", op);
    }
}

double matrix_min(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot perform minimum NULL matrix\n");
        return 0;
    }

    const int size = X->rows * X->cols;
    double min = X->data[0];
    for (int i = 1; i < size; i++) {
        if (X->data[i] < min) {
            min = X->data[i];
        }
    }

    return min;
}

double matrix_max(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot perform maximum NULL matrix\n");
        return 0;
    }

    const int size = X->rows * X->cols;
    double max = X->data[0];
    for (int i = 1; i < size; i++) {
        if (X->data[i] > max) {
            max = X->data[i];
        }
    }

    return max;
}

double matrix_sum(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot calculate sum of NULL matrix\n");
        return 0;
    }

    double sum=0;
    const int size = X->rows * X->cols;
    for (int i = 0; i < size; i++) {
        sum += X->data[i];
    }

    return sum;
}

double matrix_mean(const Matrix *X) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot calculate mean of NULL matrix\n");
        return 0;
    }

    double sum = 0;
    const int size = X->rows * X->cols;
    for (int i = 0; i < size; i++) {
        sum += X->data[i];
    }

    return sum / size;
}

double matrix_col_min(const Matrix *X, const int col) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot calculate column min of NULL matrix\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return 0;
    }

    const int n = X->rows;
    const int stride = X->cols;
    double min = X->data[col];
    for (int i = 1; i < n; i++) {
        const double val = X->data[i * stride + col];
        if (val < min) min = val;
    }

    return min;
}
double matrix_col_max(const Matrix *X, const int col) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot calculate column max of NULL matrix\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return 0;
    }

    const int n = X->rows;
    const int stride = X->cols;
    double max = X->data[col];
    for (int i = 1; i < n; i++) {
        const double val = X->data[i * stride + col];
        if (val > max) max = val;
    }

    return max;
}

double matrix_col_sum(const Matrix *X, const int col) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot calculate column sum of NULL matrix\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return 0;
    }

    double sum = 0;
    const int stride = X->cols;
    for (int i = 0; i < X->rows; i++) {
        sum += X->data[i * stride + col];
    }

    return sum;
}

double matrix_col_mean(const Matrix *X, const int col) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot calculate column mean of NULL matrix\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return 0;
    }

    double sum = 0;
    const int stride = X->cols;
    for (int i = 0; i < X->rows; i++) {
        sum += X->data[i * stride + col];
    }

    return sum / X->rows;
}

double matrix_col_std(const Matrix *X, const int col, const int ddof) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot calculate standard deviation of NULL matrix\n");
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return 0;
    }
    if (ddof != 0 && ddof != 1) {
        fprintf(stderr, "\nError: Property 'ddof' must be 0 or 1\n");
    }

    const int n = X->rows;
    const int stride = X->cols;
    double mean = 0;

    for (int i = 0; i < n; i++) {
        mean += X->data[i * stride + col];
    }
    mean /= n;

    double var = 0;
    for (int i = 0; i < n; i++) {
        const double diff = X->data[i * stride + col] - mean;
        var += diff * diff;
    }

    if (ddof == 0) {
        var /= n;
    } else {
        var /= n - 1;
    }
    return sqrt(var);
}

double matrix_col_dot_product(const Matrix *A, const int col_A, const Matrix *B, const int col_B) {
    if (!A || !B) {
        fprintf(stderr, "\nError: Cannot compute dot product with NULL matrix\n");
        return 0;
    }
    if (A->rows != B->rows) {
        fprintf(stderr, "\nError: Row dimensions must match for dot product\n");
        return 0;
    }
    if (col_A < 0 || col_A >= A->cols) {
        fprintf(stderr, "\nError: Index col_A out of bounds\n");
        return 0;
    }
    if (col_B < 0 || col_B >= B->cols) {
        fprintf(stderr, "\nError: Index col_B out of bounds\n");
        return 0;
    }

    double sum = 0;
    const int n = A->rows;
    const int stride_A = A->cols;
    const int stride_B = B->cols;

    for (int i = 0; i < n; i++) {
        sum += A->data[i * stride_A + col_A] * B->data[i * stride_B + col_B];
    }

    return sum;
}

void matrix_apply_col(const Matrix *X, const int col, double (*func)(double)) {
    if (!X) {
        fprintf(stderr, "\nError: Cannot apply function to NULL matrix\n");
        return;
    }
    if (!func) {
        fprintf(stderr, "\nError: Function pointer is NULL\n");
        return;
    }
    if (col < 0 || col >= X->cols) {
        fprintf(stderr, "\nError: Index out of bounds\n");
        return;
    }

    const int stride = X->cols;
    for (int i = 0; i < X->rows; i++) {
        X->data[i * stride + col] = func(X->data[i * stride + col]);
    }
}
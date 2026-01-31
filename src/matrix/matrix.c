#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"

Matrix *matrix_create(const int rows, const int cols) {
    if (rows < 1 || cols < 1) {
        CUSTOM_ERROR("Invalid matrix dimensions");
        return NULL;
    }
    Matrix *X = malloc(sizeof(Matrix));
    if (!X) {
        ALLOCATION_ERROR();
        return NULL;
    }

    X->rows = rows;
    X->cols = cols;
    X->data = calloc(rows * cols, sizeof(double));

    return X;
}

Matrix *matrix_copy(const Matrix *X) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return NULL;
    }

    Matrix* copy = matrix_create(X->rows, X->cols);
    if (!copy) {
        ALLOCATION_ERROR();
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
        NULL_MATRIX_ERROR();
    }
}

double matrix_get(const Matrix *X, const int i, const int j) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return 0;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        INDEX_ERROR();
        return 0;
    }
    return X->data[i * X->cols + j];
}

void matrix_set(const Matrix *X, const int i, const int j, const double value) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        INDEX_ERROR();
        return;
    }
    X->data[i * X->cols + j] = value;
}

// BUG: fix BOM check
Matrix *read_csv(const char *path, const char separator, const int has_header) {
    if (has_header < 0 || has_header > 1) {
        CUSTOM_ERROR("Property 'has_header' must be 0 or 1");
        return NULL;
    }
    FILE *file = fopen(path, "r");
    if (!file) {
        CUSTOM_ERROR("File %s not found", path);
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
        CUSTOM_ERROR("Empty CSV file");
        fclose(file);
        return NULL;
    }

    Matrix *X = matrix_create(rows, cols);
    if (!X) {
        ALLOCATION_ERROR();
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
            char *endptr;
            errno = 0;
            double val = strtod(token, &endptr);

            if (errno != 0 || endptr == token) {
                CUSTOM_WARNING("Invalid element at [%d,%d], set to 0", i, j);
                val = 0;
            }

            X->data[i * X->cols + j] = val;
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
        NULL_MATRIX_ERROR();
        return;
    }

    const size_t buf_size = X->rows * X->cols * 32 + 64;
    char *buf = malloc(buf_size);
    if (!buf) {
        ALLOCATION_ERROR();
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
        NULL_MATRIX_ERROR();
        return;
    }
    if (num < 1 || num > X->rows) {
        INDEX_ERROR();
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
        NULL_MATRIX_ERROR();
        return;
    }
    if (num > X->rows || num < 1) {
        INDEX_ERROR();
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
        NULL_MATRIX_ERROR();
        return;
    }

    printf("(%d, %d)", X->rows, X->cols);
}

double matrix_size(const Matrix *X) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return 0;
    }

    return X->rows*X->cols;
}

Matrix *matrix_transpose(const Matrix *X) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return NULL;
    }

    Matrix* transposed_matrix = matrix_create(X->cols, X->rows);
    if (!transposed_matrix) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int i = 0; i < X->rows; i++) {
        for (int j = 0; j < X->cols; j++) {
            transposed_matrix->data[j * transposed_matrix->cols + i] =X->data[i * X->cols + j];
        }
    }

    return transposed_matrix;
}

Matrix *matrix_slice(const Matrix *X, const int i_start, const int i_end, const int j_start, const int j_end) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return NULL;
    }
    if (i_start < 0 || i_end < 0 || i_start >= X->rows || i_end > X->rows || i_start >= i_end) {
        INDEX_ERROR();
        return NULL;
    }
    if (j_start < 0 || j_end < 0 || j_start >= X->cols || j_end > X->cols || j_start >= j_end) {
        INDEX_ERROR();
        return NULL;
    }

    const int rows = i_end - i_start;
    const int cols = j_end - j_start;
    Matrix* slice = matrix_create(rows, cols);
    if (!slice) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            slice->data[i * slice->cols + j] = X->data[(i_start + i) * X->cols + (j_start + j)];
        }
    }
    return slice;
}

Matrix *matrix_slice_rows(const Matrix *X, const int start, const int end) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return NULL;
    }
    if (start < 0 || end < 0 || start >= X->rows || end > X->rows || start >= end) {
        INDEX_ERROR();
        return NULL;
    }

    const int rows = end - start;
    Matrix* slice = matrix_create(rows, X->cols);
    if (!slice) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        memcpy(slice->data + i * X->cols, X->data + (start + i) * X->cols, sizeof(double) * X->cols);
    }
    return slice;
}

Matrix *matrix_slice_cols(const Matrix *X, const int start, const int end) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return NULL;
    }
    if (start < 0 || end < 0 || start >= X->cols || end > X->cols || start >= end) {
        INDEX_ERROR();
        return NULL;
    }

    const int cols = end - start;
    Matrix* slice = matrix_create(X->rows, cols);
    if (!slice) {
        ALLOCATION_ERROR();
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
        NULL_MATRIX_ERROR();
        return NULL;
    }
    if (A->rows != B->rows) {
        CUSTOM_ERROR("Matrix row dimensions must match");
        return NULL;
    }

    const int cols = A->cols + B->cols;
    Matrix* C = matrix_create(A->rows, cols);
    if (!C) {
        ALLOCATION_ERROR();
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
        NULL_MATRIX_ERROR();
        return NULL;
    }
    if (A->cols != B->cols || A->rows != B->rows) {
        CUSTOM_ERROR("Matrix dimensions must match");
        return NULL;
    }

    Matrix* C = matrix_create(A->rows, A->cols);
    if (!C) {
        ALLOCATION_ERROR();
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
                    CUSTOM_WARNING("Division by zero detected at [%d,%d], set to 0", i / A->cols, i % A->cols);
                    C->data[i] = 0;
                } else {
                    C->data[i] = A->data[i] / B->data[i];
                }
            }
            break;
        default:
            CUSTOM_ERROR("Invalid operator");
            matrix_free(C);
            return NULL;
    }
    return C;
}

// TODO: try faster multiply (strassen? winograd)
Matrix *matrix_multiplication(const Matrix *A, const Matrix *B) {
    if (!A || !B) {
        NULL_MATRIX_ERROR();
        return NULL;
    }
    if (A->cols != B->rows) {
        CUSTOM_ERROR("Incompatible dimensions for multiplication");
        return NULL;
    }

    Matrix* C = matrix_create(A->rows, B->cols);
    if (!C) {
        ALLOCATION_ERROR();
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
        NULL_MATRIX_ERROR();
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
                CUSTOM_ERROR("Division by zero is not allowed");
                return;
            }
            for (int i = 0; i < size; i++)
                X->data[i] /= scalar;
            break;
        default:
            CUSTOM_ERROR("Invalid operator");
    }
}

double matrix_min(const Matrix *X) {
    if (!X) {
        NULL_MATRIX_ERROR();
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
        NULL_MATRIX_ERROR();
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
        NULL_MATRIX_ERROR();
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
        NULL_MATRIX_ERROR();
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
        NULL_MATRIX_ERROR();
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
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
        NULL_MATRIX_ERROR();
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
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
        NULL_MATRIX_ERROR();
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
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
        NULL_MATRIX_ERROR();
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
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
        NULL_MATRIX_ERROR();
        return 0;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return 0;
    }
    if (ddof != 0 && ddof != 1) {
        CUSTOM_ERROR("Property 'ddof' must be 0 or 1");
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
        NULL_MATRIX_ERROR();
        return 0;
    }
    if (A->rows != B->rows) {
        CUSTOM_ERROR("Row dimensions must match for dot product");
        return 0;
    }
    if (col_A < 0 || col_A >= A->cols || col_B < 0 || col_B >= B->cols) {
        INDEX_ERROR();
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
        NULL_MATRIX_ERROR();
        return;
    }
    if (!func) {
        CUSTOM_ERROR("Function pointer is NULL");
        return;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return;
    }

    const int stride = X->cols;
    for (int i = 0; i < X->rows; i++) {
        X->data[i * stride + col] = func(X->data[i * stride + col]);
    }
}

Matrix *vector_to_matrix(const Vector *x) {
    if (!x) {
        NULL_VECTOR_ERROR();
        return NULL;
    }

    Matrix *X = matrix_create(x->dim, 1);
    if (!X) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int i = 0; i < x->dim; i++) {
        X->data[i] = x->data[i];
    }

    return X;
}

Vector *matrix_col_to_vector(const Matrix *X, const int col) {
    if (!X) {
        NULL_MATRIX_ERROR();
        return NULL;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return NULL;
    }

    Vector* x = vector_create(X->rows);
    if (!x) {
        ALLOCATION_ERROR();
        return NULL;
    }

    Matrix *X_col = matrix_slice_cols(X, col, col+1);
    if (!X_col) {
        ALLOCATION_ERROR();
        vector_free(x);
        return NULL;
    }

    memcpy(x->data, X_col->data, sizeof(double) * x->dim);
    matrix_free(X_col);

    return x;
}
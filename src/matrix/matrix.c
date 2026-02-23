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
    if (!X->data) {
        ALLOCATION_ERROR();
        matrix_free(X);
        return NULL;
    }

    return X;
}

Matrix *matrix_copy(const Matrix *X) {
    if (!X) {
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
    }
}

double matrix_get(const Matrix *X, const int i, const int j) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        INDEX_ERROR();
        return NAN;
    }
    return X->data[i * X->cols + j];
}

void matrix_set(Matrix *X, const int i, const int j, const double value) {
    if (!X) {
        NULL_ERROR("Matrix");
        return;
    }
    if (i >= X->rows || j >= X->cols || i < 0 || j < 0) {
        INDEX_ERROR();
        return;
    }
    X->data[i * X->cols + j] = value;
}

static int read_line(char **buf, size_t *cap, FILE *fp) {
    size_t len = 0;
    if (*buf == NULL) {
        *cap = 1024;
        *buf = malloc(*cap);
        if (!*buf) return -1;
    }
    while (1) {
        if (fgets(*buf + len, (int)(*cap - len), fp) == NULL)
            return len > 0 ? (int)len : -1;
        len += strlen(*buf + len);
        if (len > 0 && (*buf)[len - 1] == '\n')
            return (int)len;
        if (feof(fp))
            return (int)len;
        *cap *= 2;
        char *tmp = realloc(*buf, *cap);
        if (!tmp) return -1;
        *buf = tmp;
    }
}

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

    char *line = NULL;
    size_t line_cap = 0;

    const int c1 = fgetc(file);
    const int c2 = fgetc(file);
    const int c3 = fgetc(file);
    if (!(c1 == 0xEF && c2 == 0xBB && c3 == 0xBF)) {
        rewind(file);
    }

    if (has_header == 1) {
        read_line(&line, &line_cap, file);
    }

    int rows = 0;
    int cols = 0;

    while(read_line(&line, &line_cap, file) != -1) {
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
        free(line);
        fclose(file);
        return NULL;
    }

    Matrix *X = matrix_create(rows, cols);
    if (!X) {
        ALLOCATION_ERROR();
        free(line);
        fclose(file);
        return NULL;
    }

    rewind(file);
    {
        const int b1 = fgetc(file);
        const int b2 = fgetc(file);
        const int b3 = fgetc(file);
        if (!(b1 == 0xEF && b2 == 0xBB && b3 == 0xBF)) {
            rewind(file);
        }
    }
    if (has_header == 1) {
        read_line(&line, &line_cap, file);
    }

    int i = 0;
    while(read_line(&line, &line_cap, file) != -1 && i < rows) {
        int j = 0;
        const char sep[2] = {separator, '\0'};
        char *saveptr;
        const char *token = strtok_r(line, sep, &saveptr);
        while(token && j < cols) {
            char *endptr;
            errno = 0;
            double val = strtod(token, &endptr);

            if (errno != 0 || endptr == token) {
                CUSTOM_WARNING("Invalid element at [%d,%d], set to 0", i, j);
                val = 0;
            }

            X->data[i * X->cols + j] = val;
            token = strtok_r(NULL, sep, &saveptr);
            j++;
        }
        i++;
    }

    free(line);
    fclose(file);
    return X;
}

void matrix_print(const Matrix *X) {
    if (!X) {
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
        return;
    }

    printf("(%d, %d)", X->rows, X->cols);
}

double matrix_size(const Matrix *X) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }

    return X->rows*X->cols;
}

Matrix *matrix_transpose(Matrix *X, const int inplace) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (inplace != 0 && inplace != 1) {
        CUSTOM_ERROR("Property 'inplace' must be 0 or 1");
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

    if (inplace == 1) {
        matrix_free(X);
    }

    return transposed_matrix;
}

Matrix *matrix_inverse(Matrix *X, const int inplace) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (X->rows != X->cols) {
        CUSTOM_ERROR("Matrix must be square to invert");
        return NULL;
    }
    if (inplace != 0 && inplace != 1) {
        CUSTOM_ERROR("Property 'inplace' must be 0 or 1");
        return NULL;
    }

    const int n = X->rows;

    Matrix *A = matrix_create(n, n);
    if (!A) {
        ALLOCATION_ERROR();
        return NULL;
    }
    Matrix *I = matrix_create(n, n);
    if (!I) {
        ALLOCATION_ERROR();
        matrix_free(A);
        return NULL;
    }

    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            A->data[r * n + c] = X->data[r * n + c];
            I->data[r * n + c] = r == c ? 1.0 : 0.0;
        }
    }

    for (int col = 0; col < n; col++) {

        int pivot = col;
        double max_val = fabs(A->data[col * n + col]);

        for (int r = col + 1; r < n; r++) {
            const double val = fabs(A->data[r * n + col]);
            if (val > max_val) {
                max_val = val;
                pivot = r;
            }
        }

        if (max_val < 1e-12) {
            CUSTOM_ERROR("Matrix is singular (not invertible)");
            matrix_free(A);
            matrix_free(I);
            return NULL;
        }

        if (pivot != col) {
            for (int c = 0; c < n; c++) {
                double tmp = A->data[col * n + c];
                A->data[col * n + c] = A->data[pivot * n + c];
                A->data[pivot * n + c] = tmp;

                tmp = I->data[col * n + c];
                I->data[col * n + c] = I->data[pivot * n + c];
                I->data[pivot * n + c] = tmp;
            }
        }

        const double pivot_val = A->data[col * n + col];
        for (int c = 0; c < n; c++) {
            A->data[col * n + c] /= pivot_val;
            I->data[col * n + c] /= pivot_val;
        }

        for (int r = 0; r < n; r++) {
            if (r == col) continue;

            const double factor = A->data[r * n + col];

            for (int c = 0; c < n; c++) {
                A->data[r * n + c] -= factor * A->data[col * n + c];
                I->data[r * n + c] -= factor * I->data[col * n + c];
            }
        }
    }

    matrix_free(A);
    if (inplace == 1) {
        matrix_free(X);
    }
    return I;
}

Matrix *matrix_slice(const Matrix *X, const int i_start, const int i_end, const int j_start, const int j_end) {
    if (!X) {
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
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

void matrix_scalar_arithmetic(Matrix *X, const double scalar, const char op) {
    if (!X) {
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Matrix");
        return NAN;
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
        NULL_ERROR("Matrix");
        return NAN;
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
        NULL_ERROR("Matrix");
        return NAN;
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
        NULL_ERROR("Matrix");
        return NAN;
    }

    double sum = 0;
    const int size = X->rows * X->cols;
    for (int i = 0; i < size; i++) {
        sum += X->data[i];
    }

    return sum / size;
}

double matrix_row_min(const Matrix *X, const int row) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (row < 0 || row >= X->rows) {
        INDEX_ERROR();
        return NAN;
    }

    const int n = X->cols;
    double min = X->data[row * X->cols];
    for (int i = 1; i < n; i++) {
        const double val = X->data[row * X->cols + i];
        if (val < min) min = val;
    }

    return min;
}

double matrix_row_max(const Matrix *X, const int row) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (row < 0 || row >= X->rows) {
        INDEX_ERROR();
        return NAN;
    }

    const int n = X->cols;
    double max = X->data[row * X->cols];
    for (int i = 1; i < n; i++) {
        const double val = X->data[row * X->cols + i];
        if (val > max) max = val;
    }

    return max;
}

double matrix_row_sum(const Matrix *X, const int row) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (row < 0 || row >= X->rows) {
        INDEX_ERROR();
        return NAN;
    }

    double sum = 0;
    for (int i = 0; i < X->cols; i++) {
        sum += X->data[row * X->cols + i];
    }

    return sum;
}

double matrix_row_mean(const Matrix *X, const int row) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (row < 0 || row >= X->rows) {
        INDEX_ERROR();
        return NAN;
    }

    double sum = 0;
    for (int i = 0; i < X->cols; i++) {
        sum += X->data[row * X->cols + i];
    }

    return sum / X->cols;
}

double matrix_row_std(const Matrix *X, const int row, const int ddof) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (row < 0 || row >= X->rows) {
        INDEX_ERROR();
        return NAN;
    }
    if (ddof != 0 && ddof != 1) {
        CUSTOM_ERROR("Property 'ddof' must be 0 or 1");
        return NAN;
    }

    const int n = X->cols;
    if (ddof == 1 && n < 2) {
        CUSTOM_ERROR("At least 2 elements required when ddof=1");
        return NAN;
    }
    const double mean = matrix_row_mean(X, row);

    double var = 0;
    for (int i = 0; i < n; i++) {
        const double diff = X->data[row * X->cols + i] - mean;
        var += diff * diff;
    }

    if (ddof == 0) {
        var /= n;
    } else {
        var /= n - 1;
    }
    return sqrt(var);
}

double matrix_col_min(const Matrix *X, const int col) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return NAN;
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
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return NAN;
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
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return NAN;
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
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return NAN;
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
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return NAN;
    }
    if (ddof != 0 && ddof != 1) {
        CUSTOM_ERROR("Property 'ddof' must be 0 or 1");
        return NAN;
    }

    const int n = X->rows;
    if (ddof == 1 && n < 2) {
        CUSTOM_ERROR("At least 2 elements required when ddof=1");
        return NAN;
    }
    const int stride = X->cols;
    const double mean = matrix_col_mean(X, col);

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
        NULL_ERROR("Matrix");
        return NAN;
    }
    if (A->rows != B->rows) {
        CUSTOM_ERROR("Row dimensions must match for dot product");
        return NAN;
    }
    if (col_A < 0 || col_A >= A->cols || col_B < 0 || col_B >= B->cols) {
        INDEX_ERROR();
        return NAN;
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

void matrix_apply(Matrix *X, double (*func)(double)) {
    if (!X) {
        NULL_ERROR("Matrix");
        return;
    }
    if (!func) {
        CUSTOM_ERROR("Function pointer is NULL");
        return;
    }

    const int size = X->rows * X->cols;
    for (int i = 0; i < size; i++) {
        X->data[i] = func(X->data[i]);
    }
}

void matrix_apply_col(Matrix *X, const int col, double (*func)(double)) {
    if (!X) {
        NULL_ERROR("Matrix");
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
        NULL_ERROR("Vector");
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

Vector *matrix_to_vector(const Matrix *X, const int col, const int row_start, const int row_end) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (col < 0 || col >= X->cols) {
        INDEX_ERROR();
        return NULL;
    }
    if (row_start < 0 || row_end > X->rows || row_start > row_end) {
        INDEX_ERROR();
        return NULL;
    }

    Vector* x = vector_create(row_end - row_start);
    if (!x) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int i = 0; i < row_end - row_start; i++) {
        x->data[i] = X->data[(row_start + i) * X->cols + col];
    }

    return x;
}

Matrix *matrix_one_hot(const Matrix *y, const int num_classes) {
    if (!y) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    if (y->cols != 1) {
        CUSTOM_ERROR("'y' must have exactly 1 column");
        return NULL;
    }
    if (num_classes < 2) {
        CUSTOM_ERROR("'num_classes' must be at least 2");
        return NULL;
    }

    Matrix *result = matrix_create(y->rows, num_classes);
    if (!result) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int i = 0; i < y->rows; i++) {
        const int label = (int)y->data[i];
        if (label < 0 || label >= num_classes) {
            CUSTOM_ERROR("Label %d out of range [0, %d)", label, num_classes);
            matrix_free(result);
            return NULL;
        }
        result->data[i * num_classes + label] = 1.0;
    }

    return result;
}

Matrix *matrix_shuffle_rows(Matrix *X) {
    if (!X) {
        NULL_ERROR("Matrix");
        return NULL;
    }
    Matrix *res = matrix_copy(X);
    if (!res) {
        ALLOCATION_ERROR();
        return NULL;
    }
    for (int i = res->rows - 1; i > 0; i--) {
        const int j = (int)(pcg32_random_double() * (i + 1));
        for (int k = 0; k < res->cols; k++) {
            const double temp = res->data[i * res->cols + k];
            res->data[i * res->cols + k] = res->data[j * res->cols + k];
            res->data[j * res->cols + k] = temp;
        }
    }
    return res;
}
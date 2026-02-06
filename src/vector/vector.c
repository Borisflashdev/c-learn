#include <stdlib.h>
#include <string.h>

#include "vector.h"

#include <tgmath.h>


Vector *vector_create(const int dim) {
    if (dim < 1) {
        CUSTOM_ERROR("Invalid vector dimension");
        return NULL;
    }
    Vector *x = malloc(sizeof(Vector));
    if (!x) {
        ALLOCATION_ERROR();
        return NULL;
    }

    x->dim = dim;
    x->data = calloc(dim, sizeof(double));
    if (!x->data) {
        ALLOCATION_ERROR();
        vector_free(x);
        return NULL;
    }

    return x;
}

Vector *vector_copy(const Vector *x) {
    if (!x) {
        NULL_ERROR("Vector");
        return NULL;
    }

    Vector* copy = vector_create(x->dim);
    if (!copy) {
        ALLOCATION_ERROR();
        return NULL;
    }
    memcpy(copy->data, x->data, sizeof(double) * x->dim);

    return copy;
}

void vector_free(Vector *x) {
    if (x) {
        free(x->data);
        free(x);
    } else {
        NULL_ERROR("Vector");
    }
}

double vector_get(const Vector *x, const int i) {
    if (!x) {
        NULL_ERROR("Vector");
        return NAN;
    }
    if (i < 0 || i >= x->dim) {
        INDEX_ERROR();
        return NAN;
    }
    return x->data[i];
}

void vector_set(Vector *x, const int i, const double value) {
    if (!x) {
        NULL_ERROR("Vector");
        return;
    }
    if (i < 0 || i >= x->dim) {
        INDEX_ERROR();
        return;
    }
    x->data[i] = value;
}

void vector_print(const Vector *x) {
    if (!x) {
        NULL_ERROR("Vector");
        return;
    }

    const size_t buf_size = x->dim * 32 + 64;
    char *buf = malloc(buf_size);
    if (!buf) {
        ALLOCATION_ERROR();
        return;
    }

    char *p = buf;
    p += sprintf(p, "[");
    for (int i = 0; i < x->dim; i++) {
        if (i > 0) p += sprintf(p, ", ");
        p += sprintf(p, "%.6f", x->data[i]);
    }
    p += sprintf(p, "]\n");

    fwrite(buf, 1, p - buf, stdout);
    free(buf);
}

void vector_print_head(const Vector *x, const int num) {
    if (!x) {
        NULL_ERROR("Vector");
        return;
    }

    if (num < 1 || num > x->dim) {
        INDEX_ERROR();
        return;
    }

    printf("[");
    for (int i = 0; i < num; i++) {
        if (i > 0) printf(", ");
        printf("%.6f", x->data[i]);
    }
    if (num < x->dim) {
        printf(", ...]\n");
    } else {
        printf("]\n");
    }
}

void vector_print_tail(const Vector *x, const int num) {
    if (!x) {
        NULL_ERROR("Vector");
        return;
    }

    if (num < 1 || num > x->dim) {
        INDEX_ERROR();
        return;
    }

    const int start = x->dim - num;

    printf(num == x->dim ? "[" : "[...");
    for (int i = start; i < x->dim; i++) {
        if (i > start) printf(", ");
        printf("%.6f", x->data[i]);
    }
    printf("]\n");
}

Vector *vector_arithmetic(const Vector *x, const Vector *y, const char op) {
    if (!x || !y) {
        NULL_ERROR("Vector");
        return NULL;
    }

    if (x->dim != y->dim) {
        CUSTOM_ERROR("Vector dimensions must match");
        return NULL;
    }

    Vector *z = vector_create(x->dim);
    if (!z) {
        ALLOCATION_ERROR();
        return NULL;
    }

    switch (op) {
        case '+':
            for (int i = 0; i < x->dim; i++)
                z->data[i] = x->data[i] + y->data[i];
            break;

        case '-':
            for (int i = 0; i < x->dim; i++)
                z->data[i] = x->data[i] - y->data[i];
            break;

        case '*':
            for (int i = 0; i < x->dim; i++)
                z->data[i] = x->data[i] * y->data[i];
            break;

        case '/':
            for (int i = 0; i < x->dim; i++) {
                if (y->data[i] == 0) {
                    CUSTOM_WARNING("Division by zero at index %d, set to 0", i);
                    z->data[i] = 0;
                } else {
                    z->data[i] = x->data[i] / y->data[i];
                }
            }
            break;

        default:
            CUSTOM_ERROR("Invalid operator");
            vector_free(z);
            return NULL;
    }

    return z;
}

void vector_scalar_arithmetic(Vector *x, const double scalar, const char op) {
    if (!x) {
        NULL_ERROR("Vector");
        return;
    }

    switch (op) {
        case '+':
            for (int i = 0; i < x->dim; i++)
                x->data[i] += scalar;
            break;

        case '-':
            for (int i = 0; i < x->dim; i++)
                x->data[i] -= scalar;
            break;

        case '*':
            for (int i = 0; i < x->dim; i++)
                x->data[i] *= scalar;
            break;

        case '/':
            if (scalar == 0) {
                CUSTOM_ERROR("Division by zero is not allowed");
                return;
            }
            for (int i = 0; i < x->dim; i++)
                x->data[i] /= scalar;
            break;

        default:
            CUSTOM_ERROR("Invalid operator");
    }
}

double vector_min(const Vector *x) {
    if (!x) {
        NULL_ERROR("Vector");
        return NAN;
    }

    double min = x->data[0];
    for (int i = 1; i < x->dim; i++)
        if (x->data[i] < min) min = x->data[i];

    return min;
}

double vector_max(const Vector *x) {
    if (!x) {
        NULL_ERROR("Vector");
        return NAN;
    }

    double max = x->data[0];
    for (int i = 1; i < x->dim; i++)
        if (x->data[i] > max) max = x->data[i];

    return max;
}

double vector_sum(const Vector *x) {
    if (!x) {
        NULL_ERROR("Vector");
        return NAN;
    }

    double sum = 0;
    for (int i = 0; i < x->dim; i++)
        sum += x->data[i];

    return sum;
}

double vector_mean(const Vector *x) {
    if (!x) {
        NULL_ERROR("Vector");
        return NAN;
    }

    return vector_sum(x) / x->dim;
}

double vector_std(const Vector *x, const int ddof) {
    if (!x) {
        NULL_ERROR("Vector");
        return NAN;
    }
    if (ddof != 0 && ddof != 1) {
        CUSTOM_ERROR("Property 'ddof' must be 0 or 1");
        return NAN;
    }

    const int n = x->dim;
    const double mean = vector_mean(x);

    double var = 0;
    for (int i = 0; i < n; i++) {
        const double diff = x->data[i] - mean;
        var += diff * diff;
    }

    if (ddof == 0) {
        var /= n;
    } else {
        var /= n - 1;
    }
    return sqrt(var);
}

double vector_dot_product(const Vector *x, const Vector *y) {
    if (!x || !y) {
        NULL_ERROR("Vector");
        return NAN;
    }

    if (x->dim != y->dim) {
        CUSTOM_ERROR("Vector dimensions must match for dot product");
        return NAN;
    }

    double sum = 0;
    for (int i = 0; i < x->dim; i++)
        sum += x->data[i] * y->data[i];

    return sum;
}

void vector_apply(Vector *x, double (*func)(double)) {
    if (!x) {
        NULL_ERROR("Vector");
        return;
    }
    if (!func) {
        CUSTOM_ERROR("Function pointer is NULL");
        return;
    }

    for (int i = 0; i < x->dim; i++) {
        x->data[i] = func(x->data[i]);
    }
}
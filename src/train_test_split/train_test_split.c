#include "train_test_split.h"

#include <time.h>

void train_test_split(const Matrix *X, const Vector *y, Matrix **X_train, Matrix **X_test, Vector **y_train, Vector **y_test, const double test_size, const int random_state) {
    if (!X || !X_train || !X_test) {
        NULL_ERROR("Matrix");
        return;
    }
    if (!y || !y_train || !y_test) {
        NULL_ERROR("Vector");
        return;
    }
    if (X->rows != y->dim) {
        INDEX_ERROR();
        return;
    }
    if (*X_train != NULL || *X_test != NULL || *y_train != NULL || *y_test != NULL) {
        CUSTOM_ERROR("'X_train', 'X_test', 'y_train', and 'y_test' must point to NULL");
        return;
    }
    if (test_size <= 0 || test_size >= 1) {
        CUSTOM_ERROR("'test_size' must be in range (0, 1)");
        return;
    }

    const uint64_t seed = random_state < 0 ? (uint64_t)time(NULL) : (uint64_t)random_state;
    pcg32_seed(seed);

    Vector *indices = vector_create(X->rows);
    if (!indices) {
        ALLOCATION_ERROR();
        return;
    }
    for (int i = 0; i < indices->dim; i++) {
        vector_set(indices, i, i);
    }
    vector_shuffle(indices);

    const int te_size = (int)(X->rows * test_size);
    const int tr_size = X->rows - te_size;

    Matrix *X_train_set = matrix_create(tr_size, X->cols);
    Matrix *X_test_set = matrix_create(te_size, X->cols);
    Vector *y_train_set = vector_create(tr_size);
    Vector *y_test_set = vector_create(te_size);
    if (!X_train_set || !X_test_set || !y_train_set || !y_test_set) {
        ALLOCATION_ERROR();
        matrix_free(X_train_set);
        matrix_free(X_test_set);
        vector_free(y_train_set);
        vector_free(y_test_set);
        vector_free(indices);
        return;
    }

    int train_idx = 0, test_idx = 0;
    for (int i = 0; i < X->rows; i++) {
        const int in = (int)vector_get(indices, i);
        if (i < tr_size) {
            for (int j = 0; j < X->cols; j++) {
                matrix_set(X_train_set, train_idx, j, matrix_get(X, in, j));
            }
            vector_set(y_train_set, train_idx, vector_get(y, in));
            train_idx++;
        } else {
            for (int j = 0; j < X->cols; j++) {
                matrix_set(X_test_set, test_idx, j, matrix_get(X, in, j));
            }
            vector_set(y_test_set, test_idx, vector_get(y, in));
            test_idx++;
        }
    }

    *X_train = X_train_set;
    *X_test = X_test_set;
    *y_train = y_train_set;
    *y_test = y_test_set;
    vector_free(indices);
}

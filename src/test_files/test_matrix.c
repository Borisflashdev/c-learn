// Import the necessary packages
#include "../matrix/matrix.h"

void test_matrix() {
    // Create and populate a 3x3 matrix
    Matrix *A = matrix_create(3, 3);
    Matrix *B = matrix_create(3, 3);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix_set(A, i, j, i * 3 + j + 1); // [1..9]
            matrix_set(B, i, j, (i * 3 + j + 1) * 2); // [2..18]
        }
    }

    // Basic info
    matrix_shape(A);
    printf("Size: %.0f | Min: %.2f | Max: %.2f | Mean: %.2f\n", matrix_size(A), matrix_min(A), matrix_max(A), matrix_mean(A));

    // Arithmetic and multiplication
    Matrix *sum = matrix_arithmetic(A, B, '+');
    Matrix *prod = matrix_multiplication(A, B);
    matrix_print(sum);
    matrix_print(prod);

    // Transpose and scalar operations
    Matrix *T = matrix_transpose(A, 0);
    matrix_scalar_arithmetic(T, 2, '*');
    matrix_print(T);

    // Slicing
    Matrix *slice = matrix_slice(A, 0, 2, 0, 2);
    matrix_print(slice);

    // Column statistics
    printf("Col 0 — Min: %.2f | Max: %.2f | Mean: %.2f | Std: %.2f\n", matrix_col_min(A, 0), matrix_col_max(A, 0), matrix_col_mean(A, 0), matrix_col_std(A, 0, 0));

    // Vector <-> Matrix conversion
    Vector *v = matrix_to_vector(A, 1, 0, 3);
    vector_print(v);
    Matrix *from_v = vector_to_matrix(v);
    matrix_print(from_v);

    // Print utilities
    matrix_print_head(B, 2);
    matrix_print_tail(B, 2);

    // Cleanup
    matrix_free(A);
    matrix_free(B);
    matrix_free(T);
    matrix_free(sum);
    matrix_free(prod);
    matrix_free(slice);
    matrix_free(from_v);
    vector_free(v);
}
// Import the necessary packages
#include "../vector/vector.h"

void test_vector() {
    // Create and populate vectors
    Vector *a = vector_create(5);
    Vector *b = vector_create(5);
    for (int i = 0; i < 5; i++) {
        vector_set(a, i, i + 1);       // [1, 2, 3, 4, 5]
        vector_set(b, i, (i + 1) * 2); // [2, 4, 6, 8, 10]
    }

    // Basic arithmetic between vectors
    Vector *sum = vector_arithmetic(a, b, '+');
    Vector *prod = vector_arithmetic(a, b, '*');
    vector_print(sum);
    vector_print(prod);

    // Scalar operations (in-place)
    Vector *c = vector_copy(a);
    vector_scalar_arithmetic(c, 10, '*');
    vector_print(c);

    // Statistical functions
    printf("Min: %.2f | Max: %.2f | Mean: %.2f | Std: %.2f\n", vector_min(a), vector_max(a), vector_mean(a), vector_std(a, 0));

    // Dot product
    printf("Dot product: %.2f\n", vector_dot_product(a, b));

    // Print utilities
    vector_print_head(b, 3);
    vector_print_tail(b, 3);

    // Cleanup
    vector_free(a);
    vector_free(b);
    vector_free(c);
    vector_free(sum);
    vector_free(prod);
}
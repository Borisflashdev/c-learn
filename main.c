#include <stdio.h>
#include "src/matrix/matrix.h"

int main() {
    Matrix *m = read_csv("test.csv", ',', 0);
    print_matrix(m);
    printf("\n\n");
    Matrix *m2 = slice_cols(m, 4,5);
    print_matrix(m2);
    printf("\n\n");
    Matrix *m3 = matrix_concat(m, m);
    print_matrix(m3);
    return 0;
}

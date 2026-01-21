#include <stdio.h>
#include "src/matrix/matrix.h"

int main() {
    Matrix *m = read_csv("test.csv", ',', 0);
    print_matrix(m);
    printf("\n\n");
    Matrix *m2 = slice_cols(m, 3, 4);
    print_matrix(m2);
    printf("\n\n");
    printf("%f", get(m2, 0, 0));
    return 0;
}

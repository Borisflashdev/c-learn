#include <stdio.h>
#include <stdlib.h>

#include "src/matrix/matrix.h"

int main() {
    Matrix *m = read_csv("test.csv", ',', 1);
    matrix_print(m);
    shape(m);
    printf("Size: %lf \n", size(m));
    printf("%lf \n", matrix_col_sum(m, 0));
    printf("%lf \n", matrix_col_mean(m, 0));
    printf("%lf \n", matrix_col_std(m, 0, 1));
    return 0;
}

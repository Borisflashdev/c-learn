#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/matrix/matrix.h"

int main() {
    Matrix *m = read_csv("test.csv", ',', 1);
    matrix_print(m);
    matrix_shape(m);
    printf("\n");
    matrix_set(m, 3, 3, 67676767);
    matrix_print(m);

    return 0;
}

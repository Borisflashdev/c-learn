#include <stdio.h>
#include "src/matrix/matrix.h"

int main() {
    Matrix *m = read_csv("test.csv", ';', 1);
    printf("%f", get(m, 2, 2));
    return 0;
}

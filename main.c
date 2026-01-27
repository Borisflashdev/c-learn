#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"

int main() {
    Matrix *A = read_csv("test_data.csv", ',', 0);
    matrix_print(A);
    printf("\n\n");
    Scaler *scaler = scaler_create(STANDARDIZATION, 0, A->cols);
    scaler_fit_transform(scaler, A);
    matrix_print(A);
    printf("\n\n");
    scaler_inverse_transform(scaler, A);
    matrix_print(A);
    return 0;
}

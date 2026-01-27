#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"

int main() {
    Matrix *A = read_csv("test_data.csv", ',', 0);
    Scaler *scaler = scaler_create(MEAN_NORMALIZATION, 0, A->cols);
    scaler_fit_transform(scaler, A);
    matrix_print(A);
    printf("%lf", matrix_get(A, 5, 2));
    return 0;
}

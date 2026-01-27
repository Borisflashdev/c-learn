#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"

int main() {
    // Import DataFrame
    Matrix *df = read_csv("test_data.csv", ',', 0);

    // Split DataFrame in X_train, y_train, X_test, y_test
    Matrix *X_train = matrix_slice(df, 0, 15, 0, df->cols-1);
    Matrix *X_test = matrix_slice(df, 15, df->rows, 0, df->cols-1);
    Matrix *y_train = matrix_slice(df, 0, 15, df->cols-1, df->cols);
    Matrix *y_test = matrix_slice(df, 15, df->rows, df->cols-1, df->cols);

    // Free DataFrame
    free(df);
    return 0;
}

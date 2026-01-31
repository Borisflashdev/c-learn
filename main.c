#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"

int main() {
    // Import DataFrame
    Matrix *df = read_csv("test_data.csv", ',', 0);

    // Split DataFrame in train and test
    Matrix *train = matrix_slice_rows(df, 0, 15);
    Matrix *test = matrix_slice_rows(df, 15, df->rows);

    // Split train into features and target
    Matrix *X_train = matrix_slice_cols(train, 0, train->cols-1);
    Vector *y_train = matrix_to_vector(train, train->cols-1);

    // Split test into features and target
    Matrix *X_test = matrix_slice_cols(test, 0, test->cols-1);
    Vector *y_test = matrix_to_vector(test, test->cols-1);

    // Free all
    matrix_free(df);
    matrix_free(train);
    matrix_free(test);
    matrix_free(X_train);
    matrix_free(X_test);
    vector_free(y_train);
    vector_free(y_test);
    return 0;
}

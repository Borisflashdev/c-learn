#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/linear_regression/linear_regression.h"
#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"

int main() {
    // Import DataFrame
    Matrix *df = read_csv("test_data2.csv", ',', 0);

    Matrix *train = matrix_slice_rows(df, 0, 15);
    Matrix *test = matrix_slice(df, 15, df->rows, 0, df->cols-1);

    Matrix *X = matrix_slice_cols(train, 0, train->cols-1);
    Vector *y = matrix_to_vector(train, train->cols-1);

    LinearRegression *model = linear_regression_create(X->cols, 1);
    linear_regression_fit(model, X, y);
    Vector *pre = linear_regression_predict(model, test);
    vector_print(pre);

    return 0;
}

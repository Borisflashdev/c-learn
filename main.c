#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/linear_regression/linear_regression.h"
#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"

int main() {
    // Import DataFrame
    Matrix *df = read_csv("test_data.csv", ',', 0);

    Matrix *test = matrix_slice(df, 15, df->rows, 0, df->cols-1);
    Matrix *X = matrix_slice(df, 0, 15, 0, df->cols-1);
    Vector *y = matrix_to_vector(df, X->cols, 0, 15);

    LinearRegression *model = linear_regression_create(X->cols, 1);
    linear_regression_fit(model, X, y);
    Vector *prediction = linear_regression_predict(model, test);

    return 0;
}
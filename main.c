#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "src/linear_regression/linear_regression.h"
#include "src/logistic_regression/logistic_regression.h"
#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"
#include "src/sgdregression/sgdregression.h"

int main() {
    // Import DataFrame
    Matrix *df = read_csv("test_data.csv", ',', 0);

    Matrix *X = matrix_slice_cols(df, 0, df->cols-1);
    Vector *y = matrix_to_vector(df, df->cols-1, 0, df->rows);

    LogisticRegression *model = logistic_regression_create(X->cols, 1, -1, 0.5, NO_PENALTY);
    logistic_regression_fit(model, X, y, 0.01, 1000, NAN, NAN);

    Matrix *data = read_csv("my_apartment.csv", ',', 0);

    Vector *predict = logistic_regression_predict(model, data);
    vector_print(predict);

    return 0;
}

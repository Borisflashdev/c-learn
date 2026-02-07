#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "src/linear_regression/linear_regression.h"
#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"
#include "src/sgdregression/sgdregression.h"

int main() {
    // Import DataFrame
    Matrix *df = read_csv("apartment_prices.csv", ',', 0);

    Matrix *X = matrix_slice_cols(df, 0, df->cols-1);
    Vector *y = matrix_to_vector(df, df->cols-1, 0, df->rows);

    Scaler *scaler = scaler_create(MIN_MAX_NORMALIZATION, 0, X->cols);
    scaler_fit_transform(scaler, X);

    Matrix *my_data = read_csv("my_apartment.csv", ',', 0);
    scaler_transform(scaler, my_data);

    SGDRegression *model = sgd_regression_create(X->cols, 1, -1, NO_PENALTY);
    sgd_regression_fit(model, X, y, 0.001, 1000, NAN,NAN);

    Vector *prediction = sgd_regression_predict(model, my_data);
    vector_print(prediction);

    matrix_free(df);
    matrix_free(X);
    vector_free(y);
    scaler_free(scaler);
    matrix_free(my_data);
    sgd_regression_free(model);
    vector_free(prediction);

    return 0;
}

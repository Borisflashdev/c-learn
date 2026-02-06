#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/linear_regression/linear_regression.h"
#include "src/matrix/matrix.h"
#include "src/scaler/scaler.h"

int main() {
    // Import DataFrame
    Matrix *df = read_csv("apartment_prices.csv", ',', 0);

    Matrix *X = matrix_slice_cols(df, 0, df->cols-1);
    Vector *y = matrix_to_vector(df, df->cols-1, 0, df->rows);

    Scaler *scaler = scaler_create(MIN_MAX_NORMALIZATION, 0, 1);
    scaler_fit_transform(scaler, X);

    LinearRegression *model = linear_regression_create(X->cols, 1);
    linear_regression_fit(model, X, y, 0);

    Matrix *my_data = read_csv("my_apartment.csv", ',', 0);
    scaler_transform(scaler, my_data);

    Vector *prediction = linear_regression_predict(model, my_data);
    vector_print(prediction);

    return 0;
}

// Import the necessary packages
#include "../linear_regression/linear_regression.h"
#include "../matrix/matrix.h"

void test_regression() {

    // Load dataset from CSV
    Matrix *df = read_csv("apartments.csv", ',', 1);

    // Split into a training set (all rows except last 20)
    Matrix *X_train = matrix_slice(df, 0, df->rows - 20, 0, df->cols - 1);
    Vector *y_train = matrix_to_vector(df, df->cols - 1, 0, df->rows - 20);

    // Extract test features (last 20 rows)
    Matrix *X_test = matrix_slice(df, 80, df->rows, 0, df->cols - 1);

    // Fit a linear regression model using Normal Equation
    LinearRegression *model = linear_regression_create(X_train->cols, 1);
    linear_regression_fit(model, X_train, y_train, 0.01);

    // Predict on test set and display results
    Vector *prediction = linear_regression_predict(model, X_test);
    vector_print(prediction);

    // Cleanup
    matrix_free(df);
    matrix_free(X_train);
    matrix_free(X_test);
    vector_free(y_train);
    vector_free(prediction);
    linear_regression_free(model);
}

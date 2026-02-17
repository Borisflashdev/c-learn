// Import the necessary packages
#include <math.h>
#include "../logistic_regression/logistic_regression.h"
#include "../matrix/matrix.h"

void test_classification() {
    // Load dataset from CSV
    Matrix *df = read_csv("loan_approval.csv", ',', 1);

    // Split into a training set (all rows except last 20)
    Matrix *X_train = matrix_slice(df, 0, df->rows - 20, 0, df->cols - 1);
    Vector *y_train = matrix_to_vector(df, df->cols - 1, 0, df->rows - 20);

    // Extract test features (last 20 rows)
    Matrix *X_test = matrix_slice(df, 80, df->rows, 0, df->cols - 1);

    // Fit logistic regression model (batch_size=32, lr=0.001, epochs=1000, no regularization)
    LogisticRegression *model = logistic_regression_create(X_train->cols, 1, 42, 0.5, NO_PENALTY);
    logistic_regression_fit(model, X_train, y_train, 32, 0.001, 1000, NAN, NAN, 0);

    // Predict on a test set and compare against actual targets
    Vector *prediction = logistic_regression_predict(model, X_test);
    for (int i = 0; i < prediction->dim; i++) {
        printf("Target: %.0lf | Prediction: %.0lf\n", matrix_get(df, 80 + i, df->cols-1), vector_get(prediction, i));
    }

    // Cleanup
    matrix_free(df);
    matrix_free(X_train);
    matrix_free(X_test);
    vector_free(y_train);
    vector_free(prediction);
    logistic_regression_free(model);
}

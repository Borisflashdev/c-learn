// Import the necessary packages
#include "../matrix/matrix.h"
#include "../scaler/scaler.h"

void test_scaling() {
    // Load dataset from CSV
    Matrix *df = read_csv("health_risk.csv", ',', 1);

    // Standardize feature columns only (exclude target column)
    // Note: uses population std (ddof=0, divides by n), unlike sklearn which uses sample std (ddof=1, divides by n-1)
    Scaler *scaler = scaler_create(STANDARDIZATION, 0, df->cols-1);
    scaler_fit(scaler, df);
    scaler_transform(scaler, df);
    // Alternative: scaler_fit_transform(scaler, df)

    // Preview first 10 rows of scaled data
    matrix_print_head(df, 10);

    // Cleanup
    matrix_free(df);
    scaler_free(scaler);
}

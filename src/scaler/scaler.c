#include <stdlib.h>

#include "scaler.h"

Scaler *scaler_create(const ScalerType type, const int col_start, const int col_end) {
    if (type != MIN_MAX_NORMALIZATION && type != MEAN_NORMALIZATION && type != STANDARDIZATION) {
        CUSTOM_ERROR("Invalid scaler type");
        return NULL;
    }
    if (col_start < 0 || col_end < 0 || col_end <= col_start) {
        CUSTOM_ERROR("'col_start' must be >= 0 and 'col_end' must be > col_start");
        return NULL;
    }

    Scaler *scaler = malloc(sizeof(Scaler));
    if (!scaler) {
        ALLOCATION_ERROR();
        return NULL;
    }

    scaler->type = type;
    scaler->col_start = col_start;
    scaler->col_end = col_end;
    scaler->params1 = NULL;
    scaler->params2 = NULL;
    scaler->num_cols = col_end - col_start;
    scaler->fitted = 0;

    return scaler;
}

void scaler_free(Scaler *scaler) {
    if (scaler) {
        free(scaler->params1);
        free(scaler->params2);
        free(scaler);
    } else {
        NULL_ERROR("Scaler");
    }
}

void scaler_fit(Scaler *scaler, Matrix *X) {
    if (!scaler) {
        NULL_ERROR("Scaler");
        return;
    }
    if (!X) {
        NULL_ERROR("Matrix");
        return;
    }
    if (scaler->col_start > X->cols || scaler->col_end > X->cols) {
        CUSTOM_ERROR("Scaler column range exceeds matrix dimensions");
        return;
    }
    if (scaler->params1) {
        free(scaler->params1);
    }
    if (scaler->params2) {
        free(scaler->params2);
    }

    scaler->params1 = malloc(sizeof(double) * scaler->num_cols);
    if (!scaler->params1) {
        ALLOCATION_ERROR();
        return;
    }
    scaler->params2 = malloc(sizeof(double) * scaler->num_cols);
    if (!scaler->params2) {
        ALLOCATION_ERROR();
        free(scaler->params1);
        return;
    }

    int n = 0;
    for (int j = scaler->col_start; j < scaler->col_end; j++) {
        switch (scaler->type) {
            case MIN_MAX_NORMALIZATION: {
                scaler->params1[n] = matrix_col_max(X, j);
                scaler->params2[n] = matrix_col_min(X, j);
                break;
            }
            case MEAN_NORMALIZATION: {
                scaler->params1[n] = matrix_col_mean(X, j);
                scaler->params2[n] = matrix_col_max(X, j) - matrix_col_min(X, j);
                break;
            }
            case STANDARDIZATION: {
                scaler->params1[n] = matrix_col_mean(X, j);
                scaler->params2[n] = matrix_col_std(X, j, 0);
                break;
            }
            default: {
                CUSTOM_ERROR("Invalid scaler type");
                free(scaler->params1);
                free(scaler->params2);
                return;
            }
        }
        n++;
    }
    scaler->fitted = 1;
}

void scaler_transform(Scaler *scaler, Matrix *X) {
    if (!scaler) {
        NULL_ERROR("Scaler");
        return;
    }
    if (!X) {
        NULL_ERROR("Matrix");
        return;
    }
    if (scaler->col_start > X->cols || scaler->col_end > X->cols) {
        CUSTOM_ERROR("Scaler column range exceeds matrix dimensions");
        return;
    }
    if (scaler->fitted == 0) {
        CUSTOM_ERROR("Scaler must be fitted before use");
        return;
    }

    int n = 0;
    for (int j = scaler->col_start; j < scaler->col_end; j++) {
        for (int i = 0; i < X->rows; i++) {
            double scaled_val = 0;
            const double x = matrix_get(X, i, j);
            switch (scaler->type) {
                case MIN_MAX_NORMALIZATION: {
                    const double max = scaler->params1[n];
                    const double min = scaler->params2[n];
                    const double range = max - min;
                    scaled_val = range == 0 ? 0 : (x - min) / range;
                    break;
                }
                case MEAN_NORMALIZATION: {
                    const double mean = scaler->params1[n];
                    const double diff = scaler->params2[n];
                    scaled_val = diff == 0 ? 0 : (x - mean) / diff;
                    break;
                }
                case STANDARDIZATION: {
                    const double mean = scaler->params1[n];
                    const double std = scaler->params2[n];
                    scaled_val = std == 0 ? 0 : (x - mean) / std;
                    break;
                }
                default: {
                    CUSTOM_ERROR("Invalid scaler type");
                    return;
                }
            }
            matrix_set(X, i, j, scaled_val);
        }
        n++;
    }
}

void scaler_fit_transform(Scaler *scaler, Matrix *X) {
    scaler_fit(scaler, X);
    if (scaler->params1 && scaler->params2) {
        scaler_transform(scaler, X);
    } else {
        ALLOCATION_ERROR();
    }
}

void scaler_inverse_transform(Scaler *scaler, Matrix *X) {
    if (!scaler) {
        NULL_ERROR("Scaler");
        return;
    }
    if (!X) {
        NULL_ERROR("Matrix");
        return;
    }
    if (scaler->col_start > X->cols || scaler->col_end > X->cols) {
        CUSTOM_ERROR("Scaler column range exceeds matrix dimensions");
        return;
    }
    if (scaler->fitted == 0) {
        CUSTOM_ERROR("Scaler must be fitted before use");
        return;
    }

    int n = 0;
    for (int j = scaler->col_start; j < scaler->col_end; j++) {
        for (int i = 0; i < X->rows; i++) {
            double new_val = 0;
            const double x = matrix_get(X, i, j);
            switch (scaler->type) {
                case MIN_MAX_NORMALIZATION: {
                    const double max = scaler->params1[n];
                    const double min = scaler->params2[n];
                    new_val = x * (max - min) + min;
                    break;
                }
                case MEAN_NORMALIZATION: {
                    const double mean = scaler->params1[n];
                    const double diff = scaler->params2[n];
                    new_val = x * diff + mean;
                    break;
                }
                case STANDARDIZATION: {
                    const double mean = scaler->params1[n];
                    const double std = scaler->params2[n];
                    new_val = x * std + mean;
                    break;
                }
                default: {
                    CUSTOM_ERROR("Invalid scaler type");
                    return;
                }
            }
            matrix_set(X, i, j, new_val);
        }
        n++;
    }
}
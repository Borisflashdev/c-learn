#ifndef SCALER_H
#define SCALER_H

#include "../matrix/matrix.h"

typedef enum {
    MIN_MAX_NORMALIZATION,
    MEAN_NORMALIZATION,
    STANDARDIZATION
} ScalerType;

typedef struct {
    ScalerType type;
    int col_start;
    int col_end;
    double *params1;
    double *params2;
    int num_cols;
    int fitted;
} Scaler;

Scaler *scaler_create(ScalerType type, int col_start, int col_end);
void scaler_free(Scaler *scaler);

void scaler_fit(Scaler *scaler, Matrix *X);
void scaler_transform(Scaler *scaler, Matrix *X);
void scaler_fit_transform(Scaler *scaler, Matrix *X);
void scaler_inverse_transform(Scaler *scaler, Matrix *X);

#endif
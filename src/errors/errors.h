#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>

#define CUSTOM_WARNING(fmt, ...) fprintf(stderr, "\nWarning: " fmt " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__)

#define CUSTOM_ERROR(fmt, ...) fprintf(stderr, "\nError: " fmt " (%s:%d)\n", ##__VA_ARGS__, __func__, __LINE__)
#define ALLOCATION_ERROR(fmt, ...) fprintf(stderr, "\nError: Memory allocation failed (%s:%d)\n", __func__, __LINE__)
#define INDEX_ERROR(fmt, ...) fprintf(stderr, "\nError: Index out of bounds (%s:%d)\n", __func__, __LINE__)

#define NULL_MATRIX_ERROR(fmt, ...) fprintf(stderr, "\nError: NULL matrix detected (%s:%d)\n", __func__, __LINE__)
#define NULL_VECTOR_ERROR(fmt, ...) fprintf(stderr, "\nError: NULL vector detected (%s:%d)\n", __func__, __LINE__)
#define NULL_SCALER_ERROR(fmt, ...) fprintf(stderr, "\nError: NULL scaler detected (%s:%d)\n", __func__, __LINE__)
#define NULL_LINEAR_REGRESSION_ERROR(fmt, ...) fprintf(stderr, "\nError: NULL linear regression detected (%s:%d)\n", __func__, __LINE__)


#endif
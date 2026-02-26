#ifndef TRAIN_TEST_SPLIT_H
#define TRAIN_TEST_SPLIT_H

#include "../matrix/matrix.h"

void train_test_split(const Matrix *X, const Vector *y, Matrix **X_train, Matrix **X_test, Vector **y_train, Vector **y_test, double test_size, int random_state);

#endif
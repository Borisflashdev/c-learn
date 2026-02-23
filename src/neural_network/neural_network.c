#include "neural_network.h"
#include "../errors/errors.h"
#include "../matrix/matrix.h"
#include "../vector/vector.h"
#include "../math_functions/math_functions.h"
#include "../random/random.h"

#include <stdlib.h>
#include <stdio.h>
#include <tgmath.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

NeuralNetwork *neural_network_create(const int input_size, const int num_layers, const LossFunction loss_function, int const random_seed) {
    if (input_size <= 0) {
        CUSTOM_ERROR("'input_size' must be at least 1");
        return NULL;
    }
    if (num_layers <= 0) {
        CUSTOM_ERROR("'num_layers' must be at least 1");
        return NULL;
    }

    NeuralNetwork *nn = malloc(sizeof(NeuralNetwork));
    if (!nn) {
        ALLOCATION_ERROR();
        return NULL;
    }
    nn->layers = calloc(num_layers, sizeof(DenseLayer *));
    if (!nn->layers) {
        ALLOCATION_ERROR();
        free(nn);
        return NULL;
    }

    nn->input_size = input_size;
    nn->random_seed = random_seed;
    nn->num_layers = num_layers;
    nn->current_num_layers = 0;
    nn->loss_function = loss_function;

    return nn;
}

void neural_network_free(NeuralNetwork *neural_network) {
    if (!neural_network) {
        NULL_ERROR("NeuralNetwork model");
        return;
    }
    for (int i = 0; i < neural_network->num_layers; i++) {
        if (neural_network->layers[i]) {
            free(neural_network->layers[i]->name);
            matrix_free(neural_network->layers[i]->coef);
            vector_free(neural_network->layers[i]->intercepts);
            free(neural_network->layers[i]);
        }
    }
    free(neural_network->layers);
    free(neural_network);
}

void neural_network_describe(NeuralNetwork *neural_network) {
    if (!neural_network) {
        NULL_ERROR("NeuralNetwork model");
        return;
    }

    printf("Neural Network Description\n");
    for (int i = 0; i < neural_network->current_num_layers; i++) {
        const DenseLayer *layer = neural_network->layers[i];

        char *activation = NULL;
        switch (layer->activation) {
            case ReLU: activation = "ReLU"; break;
            case LeakyReLU: activation = "LeakyReLU"; break;
            case SiLU: activation = "SiLU"; break;
            case Sigmoid: activation = "Sigmoid"; break;
            case Tanh: activation = "Tanh"; break;
            case Softmax: activation = "Softmax"; break;
            case Linear: activation = "Linear"; break;
            default: CUSTOM_ERROR("Unknown activation"); return;
        }

        printf("%d. %s | Units: %d | Activation: %s | Weights Dimensions: (%d, %d) | Parameters: %d\n", i, layer->name, layer->units, activation, layer->coef->rows, layer->coef->cols, layer->coef->rows * layer->coef->cols + layer->intercepts->dim);
    }

    int total_params = 0;
    for (int i = 0; i < neural_network->current_num_layers; i++) {
        total_params += neural_network->layers[i]->coef->rows * neural_network->layers[i]->coef->cols + neural_network->layers[i]->intercepts->dim;
    }
    printf("Total parameters: %d\n", total_params);
}

void neural_network_add_layer(NeuralNetwork *neural_network, const int units, const Activation activation, const Penalty penalty, const double lambda, const double ratio, const char *name) {
    if (!neural_network) {
        NULL_ERROR("NeuralNetwork model");
        return;
    }
    if (units <= 0) {
        CUSTOM_ERROR("'units' must be at least 1");
        return;
    }
    if (neural_network->current_num_layers >= neural_network->num_layers) {
        CUSTOM_ERROR("Too many layers");
        return;
    }

    switch (penalty) {
        case NO_PENALTY: {
            if (!isnan(lambda) || !isnan(ratio)) {
                CUSTOM_ERROR("'lambda' and 'ratio' are unused with NO_PENALTY, pass NAN");
                return;
            }
            break;
        }
        case L1_LASSO: {
            if (!isnan(ratio)) {
                CUSTOM_ERROR("'ratio' is unused with L1_LASSO, pass NAN");
                return;
            }
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("'lambda' must be non-negative");
                return;
            }
            break;
        }
        case L2_RIDGE: {
            if (!isnan(ratio)) {
                CUSTOM_ERROR("'ratio' is unused with L2_RIDGE, pass NAN");
                return;
            }
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("'lambda' must be non-negative");
                return;
            }
            break;
        }
        case ELASTIC_NET: {
            if (lambda < 0 || isnan(lambda)) {
                CUSTOM_ERROR("'lambda' must be non-negative");
                return;
            }
            if (ratio < 0 || ratio > 1 || isnan(ratio)) {
                CUSTOM_ERROR("'ratio' must be between 0 and 1");
                return;
            }
            break;
        }
    }

    DenseLayer *layer = malloc(sizeof(DenseLayer));
    if (!layer) {
        ALLOCATION_ERROR();
        return;
    }
    layer->name = strdup(name);
    layer->units = units;
    layer->activation = activation;
    layer->coef = matrix_create(neural_network->current_num_layers == 0 ? neural_network->input_size : neural_network->layers[neural_network->current_num_layers-1]->units, units);
    layer->intercepts =  vector_create(units);
    layer->penalty = penalty;
    layer->lambda = isnan(lambda) ? 0 : lambda;
    layer->ratio = isnan(ratio) ? 0 : ratio;

    const double limit = math_xavier(layer->coef->rows, layer->coef->cols);
    for (int i = 0; i < layer->coef->rows; i++) {
        for (int j = 0; j < layer->coef->cols; j++) {
            matrix_set(layer->coef, i, j, pcg32_random_double() * 2.0 * limit - limit);
        }
    }

    neural_network->layers[neural_network->current_num_layers++] = layer;
}

void neural_network_fit(NeuralNetwork *neural_network, Matrix *X, Matrix *y, int epochs, double learning_rate, int batch_size) {
    if (!neural_network) {
        NULL_ERROR("NeuralNetwork model");
        return;
    }
    if (!X) {
        NULL_ERROR("X matrix");
        return;
    }
    if (!y) {
        NULL_ERROR("y matrix");
        return;
    }
    if (epochs <= 0) {
        CUSTOM_ERROR("'epochs' must be at least 1");
        return;
    }
    if (learning_rate <= 0) {
        CUSTOM_ERROR("'learning_rate' must be positive");
        return;
    }
    if (batch_size <= 0 || batch_size > X->rows) {
        CUSTOM_ERROR("'batch_size' must be between 1 and X->rows");
        return;
    }
    if (neural_network->current_num_layers == 0) {
        CUSTOM_ERROR("No layers added to the network");
        return;
    }
    if (X->rows != y->rows) {
        CUSTOM_ERROR("X->rows must equal y->rows");
        return;
    }
    if (X->cols != neural_network->input_size) {
        CUSTOM_ERROR("X->cols must equal input_size");
        return;
    }

    const uint64_t seed = neural_network->random_seed < 0 ? (uint64_t)time(NULL) : (uint64_t)neural_network->random_seed;
    pcg32_seed(seed);

    const int L = neural_network->current_num_layers;
    const int N = X->rows;

    Vector *indices = vector_create(N);
    if (!indices) {
        ALLOCATION_ERROR();
        return;
    }
    for (int i = 0; i < N; i++) {
        vector_set(indices, i, i);
    }

    for (int epoch = 0; epoch < epochs; epoch++) {
        vector_shuffle(indices);
        double total_loss = 0.0;

        for (int k = 0; k < N; k += batch_size) {
            const int bs = k + batch_size > N ? N - k : batch_size;

            Matrix *X_batch = matrix_create(bs, X->cols);
            Matrix *y_batch = matrix_create(bs, y->cols);
            if (!X_batch || !y_batch) { ALLOCATION_ERROR(); vector_free(indices); return; }

            for (int i = 0; i < bs; i++) {
                const int row = (int)vector_get(indices, k + i);
                for (int j = 0; j < X->cols; j++) {
                    matrix_set(X_batch, i, j, matrix_get(X, row, j));
                }
                for (int j = 0; j < y->cols; j++) {
                    matrix_set(y_batch, i, j, matrix_get(y, row, j));
                }

            }

            Matrix *post[L + 1];
            Matrix *pre[L];
            post[0] = X_batch;

            for (int l = 0; l < L; l++) {
                const DenseLayer *layer = neural_network->layers[l];

                Matrix *Z = matrix_multiplication(post[l], layer->coef);

                for (int i = 0; i < Z->rows; i++) {
                    for (int j = 0; j < Z->cols; j++) {
                        matrix_set(Z, i, j, matrix_get(Z, i, j) + vector_get(layer->intercepts, j));
                    }
                }

                pre[l] = Z;
                Matrix *A = matrix_copy(Z);

                if (layer->activation != Softmax) {
                    for (int i = 0; i < A->rows; i++) {
                        for (int j = 0; j < A->cols; j++) {
                            double val = matrix_get(A, i, j);
                            switch (layer->activation) {
                                case ReLU: val = math_relu(val); break;
                                case LeakyReLU: val = math_leaky_relu(val); break;
                                case SiLU: val = math_silu(val); break;
                                case Sigmoid: val = math_sigmoid(val); break;
                                case Tanh: val = math_tanh(val); break;
                                default: break;
                            }
                            matrix_set(A, i, j, val);
                        }
                    }
                } else {
                    for (int i = 0; i < A->rows; i++) {
                        const double max_val = matrix_row_max(A, i);
                        double sum = 0.0;
                        for (int j = 0; j < A->cols; j++) {
                            const double e = exp(matrix_get(A, i, j) - max_val);
                            matrix_set(A, i, j, e);
                            sum += e;
                        }
                        for (int j = 0; j < A->cols; j++) {
                            matrix_set(A, i, j, matrix_get(A, i, j) / sum);
                        }
                    }
                }

                post[l + 1] = A;
            }

            for (int i = 0; i < bs; i++) {
                for (int j = 0; j < post[L]->cols; j++) {
                    const double y_hat = matrix_get(post[L], i, j);
                    const double y_true = matrix_get(y_batch, i, j);
                    switch (neural_network->loss_function) {
                        case MSE: {
                            const double diff = y_hat - y_true;
                            total_loss += diff * diff;
                            break;
                        }
                        case BinaryCrossEntropy: {
                            const double eps = 1e-15;
                            total_loss += -y_true * log(y_hat + eps) - (1.0 - y_true) * log(1.0 - y_hat + eps);
                            break;
                        }
                        case CategoricalCrossEntropy: {
                            const double eps = 1e-15;
                            total_loss += -y_true * log(y_hat + eps);
                            break;
                        }
                    }
                }
            }

            Matrix *deltas[L];
            Matrix *delta_out = matrix_create(bs, neural_network->layers[L - 1]->units);
            if (!delta_out) {
                ALLOCATION_ERROR(); vector_free(indices);
                return;
            }

            for (int i = 0; i < bs; i++) {
                for (int j = 0; j < delta_out->cols; j++) {
                    const double y_hat = matrix_get(post[L], i, j);
                    const double y_true = matrix_get(y_batch, i, j);
                    double d = y_hat - y_true;

                    if (neural_network->loss_function == MSE) {
                        const double z = matrix_get(pre[L - 1], i, j);
                        switch (neural_network->layers[L - 1]->activation) {
                            case ReLU: d *= math_derivative_relu(z); break;
                            case LeakyReLU: d *= math_derivative_leaky_relu(z); break;
                            case SiLU: d *= math_derivative_silu(z); break;
                            case Sigmoid: d *= math_derivative_sigmoid(z); break;
                            case Tanh: d *= math_derivative_tanh(z); break;
                            default: break;
                        }
                    }
                    matrix_set(delta_out, i, j, d);
                }
            }
            deltas[L - 1] = delta_out;
            for (int l = L - 2; l >= 0; l--) {
                Matrix *W_T  = matrix_transpose(neural_network->layers[l + 1]->coef, 0);
                Matrix *prop = matrix_multiplication(deltas[l + 1], W_T);
                matrix_free(W_T);

                Matrix *delta_l = matrix_create(bs, neural_network->layers[l]->units);
                if (!delta_l) {
                    ALLOCATION_ERROR(); vector_free(indices);
                    matrix_free(pre);
                    matrix_free(post);
                    return;
                }

                for (int i = 0; i < bs; i++) {
                    for (int j = 0; j < delta_l->cols; j++) {
                        const double z = matrix_get(pre[l], i, j);
                        double deriv;
                        switch (neural_network->layers[l]->activation) {
                            case ReLU: deriv = math_derivative_relu(z); break;
                            case LeakyReLU: deriv = math_derivative_leaky_relu(z); break;
                            case SiLU: deriv = math_derivative_silu(z); break;
                            case Sigmoid: deriv = math_derivative_sigmoid(z); break;
                            case Tanh: deriv = math_derivative_tanh(z); break;
                            default: deriv = 1.0; break;
                        }
                        matrix_set(delta_l, i, j, matrix_get(prop, i, j) * deriv);
                    }
                }
                matrix_free(prop);
                deltas[l] = delta_l;
            }

            for (int l = 0; l < L; l++) {
                Matrix *post_T = matrix_transpose(post[l], 0);
                Matrix *dW = matrix_multiplication(post_T, deltas[l]);
                matrix_free(post_T);

                const double lambda = neural_network->layers[l]->lambda;
                const double ratio  = neural_network->layers[l]->ratio;

                for (int i = 0; i < neural_network->layers[l]->coef->rows; i++) {
                    for (int j = 0; j < neural_network->layers[l]->coef->cols; j++) {
                        const double grad = matrix_get(dW, i, j) / bs;
                        double w = matrix_get(neural_network->layers[l]->coef, i, j);
                        switch (neural_network->layers[l]->penalty) {
                            case L2_RIDGE:
                                w -= learning_rate * (grad + lambda * w);
                                break;
                            case L1_LASSO:
                                w -= learning_rate * (grad + lambda * (w > 0 ? 1.0 : -1.0));
                                break;
                            case ELASTIC_NET: {
                                const double l1 = ratio * (w > 0 ? 1.0 : -1.0);
                                const double l2 = (1.0 - ratio) * w;
                                w -= learning_rate * (grad + lambda * (l1 + l2));
                                break;
                            }
                            default:
                                w -= learning_rate * grad;
                                break;
                        }
                        matrix_set(neural_network->layers[l]->coef, i, j, w);
                    }
                }
                matrix_free(dW);

                for (int j = 0; j < neural_network->layers[l]->intercepts->dim; j++) {
                    double db = 0.0;
                    for (int i = 0; i < bs; i++)
                        db += matrix_get(deltas[l], i, j);
                    const double b = vector_get(neural_network->layers[l]->intercepts, j);
                    vector_set(neural_network->layers[l]->intercepts, j, b - learning_rate * (db / bs));
                }
            }

            for (int l = 0; l < L; l++) matrix_free(pre[l]);
            for (int l = 1; l <= L; l++) matrix_free(post[l]);
            for (int l = 0; l < L; l++) matrix_free(deltas[l]);
            matrix_free(X_batch);
            matrix_free(y_batch);
        }

        printf("Epoch: %d | Loss: [%lf]\n", epoch + 1, total_loss / N);
    }

    vector_free(indices);
}

Matrix *neural_network_predict(NeuralNetwork *neural_network, Matrix *X) {
    if (!neural_network) {
        NULL_ERROR("NeuralNetwork model");
        return NULL;
    }
    if (!X) {
        NULL_ERROR("X matrix");
        return NULL;
    }

    Matrix *current = matrix_copy(X);
    if (!current) {
        ALLOCATION_ERROR();
        return NULL;
    }

    for (int x = 0; x < neural_network->current_num_layers; x++) {
        const DenseLayer *layer = neural_network->layers[x];

        Matrix *Z = matrix_multiplication(current, layer->coef);
        for (int i = 0; i < Z->rows; i++) {
            for (int j = 0; j < Z->cols; j++) {
                double new_val = matrix_get(Z, i, j) + vector_get(layer->intercepts, j);
                switch (layer->activation) {
                    case ReLU: new_val = math_relu(new_val); break;
                    case LeakyReLU: new_val = math_leaky_relu(new_val); break;
                    case SiLU: new_val = math_silu(new_val); break;
                    case Sigmoid: new_val = math_sigmoid(new_val); break;
                    case Tanh: new_val = math_tanh(new_val); break;
                    default: break;
                }
                matrix_set(Z, i, j, new_val);
            }
        }

        if (layer->activation == Softmax) {
            for (int i = 0; i < Z->rows; i++) {

                const double max_val = matrix_row_max(Z, i);

                double sum = 0.0;
                for (int j = 0; j < Z->cols; j++) {
                    const double e = exp(matrix_get(Z, i, j) - max_val);
                    matrix_set(Z, i, j, e);
                    sum += e;
                }

                for (int j = 0; j < Z->cols; j++) {
                    matrix_set(Z, i, j, matrix_get(Z, i, j) / sum);
                }
            }
        }

        matrix_free(current);
        current = Z;
    }

    return current;
}
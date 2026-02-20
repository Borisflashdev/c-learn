#include "neural_network.h"
#include "../errors/errors.h"
#include "../matrix/matrix.h"
#include "../vector/vector.h"
#include "../math_functions/math_functions.h"
#include "../random/random.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NeuralNetwork *neural_network_create(const int input_size, const int num_layers, const LossFunction loss_function, const int random_seed) {
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

void neural_network_add_layer(NeuralNetwork *neural_network, const int units, const Activation activation, const char *name) {
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

    const double limit = math_xavier(layer->coef->rows, layer->coef->cols);
    for (int i = 0; i < layer->coef->rows; i++) {
        for (int j = 0; j < layer->coef->cols; j++) {
            matrix_set(layer->coef, i, j, pcg32_random_double() * 2.0 * limit - limit);
        }
    }

    neural_network->layers[neural_network->current_num_layers++] = layer;
}

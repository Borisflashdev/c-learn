#include "neural_network.h"
#include "../errors/errors.h"

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

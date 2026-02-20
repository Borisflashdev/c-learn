#include "neural_networks.h"
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
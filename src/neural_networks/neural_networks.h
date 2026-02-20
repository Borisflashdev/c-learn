#ifndef NEURAL_NETWORKS_H
#define NEURAL_NETWORKS_H
#include "../matrix/matrix.h"
#include "../vector/vector.h"

typedef enum {
    BinaryCrossEntropy,
    CategoricalCrossEntropy,
    MSE
} LossFunction;

typedef enum {
    ReLU,
    LeakyReLU,
    SiLU,
    Sigmoid,
    Tanh,
    Softmax,
    Linear
} Activation;

typedef struct DenseLayer {
    char *name;
    int units;
    Matrix *coef;
    Vector *intercepts;
    Activation activation;
} DenseLayer;

typedef struct {
    int input_size;
    int random_seed;
    int num_layers;
    int current_num_layers;
    DenseLayer **layers;
    LossFunction loss_function;
} NeuralNetwork;

NeuralNetwork *neural_network_create(int input_size, int num_layers, LossFunction loss_function, int random_seed);
void neural_network_free(NeuralNetwork *neural_network);
void neural_network_describe(NeuralNetwork *neural_network);

void neural_network_add_layer(NeuralNetwork *neural_network, int units, Activation activation, const char *name);

void neural_network_fit(NeuralNetwork *neural_network, Matrix *X, Matrix *y, int epochs, double learning_rate);
Matrix *neural_network_predict(NeuralNetwork *neural_network, Matrix *X);

#endif
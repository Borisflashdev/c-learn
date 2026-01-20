#include "matrix.h"
#include <stdlib.h>
#include <string.h>

Matrix create_matrix(int rows, int cols) {
    Matrix matrix;
    matrix.rows = rows;
    matrix.cols = cols;
    matrix.data = malloc(rows * cols * sizeof(double));
    return matrix;
}

void free_matrix(Matrix *matrix) {
    free(matrix->data);
    free(matrix);
}

double get(Matrix *X, int i, int j) {
    return X->data[i * X->cols + j];
}

void set(Matrix *X, int i, int j, double value) {
    X->data[i * X->cols + j] = value;
}

Matrix *read_csv(const char *path, char separator, int has_header) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("Error opening file %s\n", path);
        return;
    }

    char line[1024];

    //--------------------Count cols--------------------//
    fgets(line, sizeof(line), file);
    int cols = 1;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == separator) {
            cols++;
        }
    }
    //--------------------Count rows--------------------//
    int rows = 1;
    while(fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) {
            rows++;
        }
    }
    if (has_header == 1) {
        rows--;
    }
    //--------------------------------------------------//

    fseek(file, 0, SEEK_SET);

    Matrix* matrix = malloc(sizeof(Matrix));
    *matrix = create_matrix(rows, cols);

    if (has_header == 1) {
        fgets(line, sizeof(line), file);
    }

    int i=0;
    while(fgets(line, sizeof(line), file)) {
        int j=0;
        char sep[2] = {separator, '\0'};
        char *token = strtok(line, sep);
        while(token) {
            double value = atof(token);
            set(matrix, i, j, value);
            token = strtok(NULL, sep);
            j++;
        }
        i++;
    }

    fclose(file);
    return matrix;
}


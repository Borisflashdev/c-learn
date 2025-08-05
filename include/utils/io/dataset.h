#ifndef DATASET_H
#define DATASET_H

typedef enum {
    TYPE_DOUBLE,
    TYPE_STRING
} DataType;

typedef struct {
    char* name;
    DataType type;
    void** data;
} Column;

typedef struct {
    Column* columns;
    int n_cols;
    int n_rows;
} DataFrame;

DataFrame* read_csv(const char* path, const char* separator);
void free_dataframe(DataFrame* df);

#endif
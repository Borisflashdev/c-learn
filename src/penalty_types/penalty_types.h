#ifndef PENALTY_TYPES_H
#define PENALTY_TYPES_H

typedef enum {
    NO_PENALTY,
    L1_LASSO,
    L2_RIDGE,
    ELASTIC_NET
} Penalty;

#endif
#ifndef A7_GRAPHCOMMON_H
#define A7_GRAPHCOMMON_H

#include <stddef.h>

#define N_VERTICES 100 

typedef struct {
    size_t base;
    size_t body;
    size_t nodes;
    size_t total;
} MemBreakdown;

#endif 
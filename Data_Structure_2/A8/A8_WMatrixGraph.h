#ifndef A8_WMATRIXGRAPH_H
#define A8_WMATRIXGRAPH_H

#include "A8_GraphCommon.h"

typedef struct {
    int  n; 
    int  edge_count; 
    int* w;
} MatrixGraph;

MatrixGraph* wm_create(int n);
void         wm_destroy(MatrixGraph* g);
void         wm_add_edge(MatrixGraph* g, int u, int v, int w);

static inline int wm_weight(const MatrixGraph* g, int u, int v) { return g->w[u * g->n + v]; }

void wm_memory(const MatrixGraph* g, MemReport* m);

#endif 
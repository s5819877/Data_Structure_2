#ifndef A7_MATRIXGRAPH_H
#define A7_MATRIXGRAPH_H

#include "A7_GraphCommon.h"

#ifndef MATRIX_CELL_TYPE
#define MATRIX_CELL_TYPE int
#endif
typedef MATRIX_CELL_TYPE MatrixCell;

typedef struct {
    int         n;
    int         edge_count;
    MatrixCell* cell;
} MatrixGraph;

MatrixGraph* mg_create(int n);
void         mg_destroy(MatrixGraph* g);

void mg_build_edge(MatrixGraph* g, int u, int v);

int mg_insert_edge(MatrixGraph* g, int u, int v, long* cmp);
int mg_delete_edge(MatrixGraph* g, int u, int v, long* cmp);
int mg_has_edge(const MatrixGraph* g, int u, int v, long* cmp);
int mg_neighbors(const MatrixGraph* g, int u, int* out, long* cmp);

int mg_peek(const MatrixGraph* g, int u, int v);

void mg_memory(const MatrixGraph* g, MemBreakdown* m);

#endif
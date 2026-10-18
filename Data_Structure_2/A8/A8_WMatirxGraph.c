#include <stdio.h>
#include <stdlib.h>
#include "A8_WMatrixGraph.h"

MatrixGraph* wm_create(int n)
{
    MatrixGraph* g = (MatrixGraph*)malloc(sizeof(MatrixGraph));
    if (!g) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    g->w = (int*)calloc((size_t)n * (size_t)n, sizeof(int));
    if (!g->w) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    g->n = n;
    g->edge_count = 0;
    return g;
}

void wm_destroy(MatrixGraph* g)
{
    if (!g) return;
    free(g->w);
    free(g);
}

void wm_add_edge(MatrixGraph* g, int u, int v, int w)
{
    g->w[u * g->n + v] = w;
    g->w[v * g->n + u] = w;
    g->edge_count++;
}

void wm_memory(const MatrixGraph* g, MemReport* m)
{
    mem_reset(m);
    mem_add(m, "그래프 구조체", sizeof(MatrixGraph));
    mem_add(m, "인접 행렬 (n x n)", (size_t)g->n * (size_t)g->n * sizeof(int));
}
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "A7_MatrixGraph.h"

MatrixGraph* mg_create(int n)
{
    MatrixGraph* g = (MatrixGraph*)malloc(sizeof(MatrixGraph));
    if (!g) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }

    g->cell = (MatrixCell*)calloc((size_t)n * (size_t)n, sizeof(MatrixCell));
    if (!g->cell) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }

    g->n = n;
    g->edge_count = 0;
    return g;
}

void mg_destroy(MatrixGraph* g)
{
    if (!g) return;
    free(g->cell);
    free(g);
}

void mg_build_edge(MatrixGraph* g, int u, int v)
{
    g->cell[u * g->n + v] = 1;
    g->cell[v * g->n + u] = 1;
    g->edge_count++;
}

int mg_insert_edge(MatrixGraph* g, int u, int v, long* cmp)
{
    (*cmp)++;
    if (g->cell[u * g->n + v]) return 0;
    g->cell[u * g->n + v] = 1;
    g->cell[v * g->n + u] = 1;
    g->edge_count++;
    return 1;
}

int mg_delete_edge(MatrixGraph* g, int u, int v, long* cmp)
{
    (*cmp)++;
    if (!g->cell[u * g->n + v]) return 0;
    g->cell[u * g->n + v] = 0;
    g->cell[v * g->n + u] = 0;
    g->edge_count--;
    return 1;
}

int mg_has_edge(const MatrixGraph* g, int u, int v, long* cmp)
{
    (*cmp)++;
    return g->cell[u * g->n + v] != 0;
}

int mg_neighbors(const MatrixGraph* g, int u, int* out, long* cmp)
{
    int k = 0;
    for (int v = 0; v < g->n; v++) {
        (*cmp)++;
        if (g->cell[u * g->n + v]) out[k++] = v;
    }
    return k;
}

int mg_peek(const MatrixGraph* g, int u, int v)
{
    return g->cell[u * g->n + v] != 0;
}

void mg_memory(const MatrixGraph* g, MemBreakdown* m)
{
    m->base = sizeof(MatrixGraph);
    m->body = (size_t)g->n * (size_t)g->n * sizeof(MatrixCell);
    m->nodes = 0;
    m->total = m->base + m->body + m->nodes;
}
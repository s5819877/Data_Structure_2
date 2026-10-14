#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "A7_ListGraph.h"

ListGraph* lg_create(int n)
{
    ListGraph* g = (ListGraph*)malloc(sizeof(ListGraph));
    if (!g) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }

    g->head = (AdjNode**)calloc((size_t)n, sizeof(AdjNode*));
    if (!g->head) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }

    g->n = n;
    g->edge_count = 0;
    g->node_count = 0;
    return g;
}

void lg_destroy(ListGraph* g)
{
    if (!g) return;
    for (int u = 0; u < g->n; u++) {
        AdjNode* p = g->head[u];
        while (p) { AdjNode* nx = p->next; free(p); p = nx; }
    }
    free(g->head);
    free(g);
}

static void push_front(ListGraph* g, int u, int v)
{
    AdjNode* p = (AdjNode*)malloc(sizeof(AdjNode));
    if (!p) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    p->vertex = v;
    p->next = g->head[u];
    g->head[u] = p;
    g->node_count++;
}

static int remove_node(ListGraph* g, int u, int v, long* c)
{
    AdjNode* prev = NULL, * cur = g->head[u];
    while (cur) {
        (*c)++;
        if (cur->vertex == v) {
            if (prev) prev->next = cur->next;
            else      g->head[u] = cur->next;
            free(cur);
            g->node_count--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

void lg_build_edge(ListGraph* g, int u, int v)
{
    push_front(g, u, v);
    push_front(g, v, u);
    g->edge_count++;
}

int lg_insert_edge(ListGraph* g, int u, int v, long* cmp)
{
    for (const AdjNode* p = g->head[u]; p; p = p->next) {
        (*cmp)++;
        if (p->vertex == v) return 0;
    }
    push_front(g, u, v);
    push_front(g, v, u);
    g->edge_count++;
    return 1;
}

int lg_delete_edge(ListGraph* g, int u, int v, long* cmp)
{
    long c = 0;
    if (!remove_node(g, u, v, &c)) { *cmp += c; return 0; }
    remove_node(g, v, u, &c);
    *cmp += c;
    g->edge_count--;
    return 1;
}

int lg_has_edge(const ListGraph* g, int u, int v, long* cmp)
{
    for (const AdjNode* p = g->head[u]; p; p = p->next) {
        (*cmp)++;
        if (p->vertex == v) return 1;
    }
    return 0;
}

int lg_neighbors(const ListGraph* g, int u, int* out, long* cmp)
{
    int k = 0;
    for (const AdjNode* p = g->head[u]; p; p = p->next) {
        (*cmp)++;
        out[k++] = p->vertex;
    }
    return k;
}

int lg_degree(const ListGraph* g, int u)
{
    int d = 0;
    for (const AdjNode* p = g->head[u]; p; p = p->next) d++;
    return d;
}

void lg_memory(const ListGraph* g, MemBreakdown* m)
{
    m->base = sizeof(ListGraph);
    m->body = (size_t)g->n * sizeof(AdjNode*);
    m->nodes = (size_t)g->node_count * sizeof(AdjNode);
    m->total = m->base + m->body + m->nodes;
}
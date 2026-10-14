#ifndef A7_LISTGRAPH_H
#define A7_LISTGRAPH_H

#include "A7_GraphCommon.h"

typedef struct AdjNode {
    int              vertex;
    struct AdjNode* next;
} AdjNode;

typedef struct {
    int       n;
    int       edge_count;
    int       node_count;
    AdjNode** head;
} ListGraph;

ListGraph* lg_create(int n);
void       lg_destroy(ListGraph* g);

void lg_build_edge(ListGraph* g, int u, int v);

int lg_insert_edge(ListGraph* g, int u, int v, long* cmp);
int lg_delete_edge(ListGraph* g, int u, int v, long* cmp);
int lg_has_edge(const ListGraph* g, int u, int v, long* cmp);
int lg_neighbors(const ListGraph* g, int u, int* out, long* cmp);

int lg_degree(const ListGraph* g, int u);

void lg_memory(const ListGraph* g, MemBreakdown* m);

#endif /* LIST_GRAPH_H */
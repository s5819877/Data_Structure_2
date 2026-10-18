#ifndef A8_WLISTGRAPH_H
#define A8_WLISTGRAPH_H

#include "A8_GraphCommon.h"

typedef struct WNode {
    int           vertex;
    int           weight;
    struct WNode* next;
} WNode;

typedef struct {
    int     n;
    int     edge_count;
    WNode** head;
} ListGraph;

ListGraph* wl_create(int n);
void       wl_destroy(ListGraph* g);
void       wl_add_edge(ListGraph* g, int u, int v, int w);

void wl_memory(const ListGraph* g, MemReport* m);

int wl_is_connected(const ListGraph* g);

#endif
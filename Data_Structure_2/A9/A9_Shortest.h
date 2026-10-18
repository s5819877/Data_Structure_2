#ifndef A9_SHORTEST_H
#define A9_SHORTEST_H

#include "A9_Graph.h"

#define INF 0x20000000

typedef struct {
    int dist[N_VERTICES][N_VERTICES];
    int next[N_VERTICES][N_VERTICES];
} AllPairs;

void floyd_warshall(const Graph* g, AllPairs* ap);

int path_from_next(const AllPairs* ap, int u, int v, int* path);

void dijkstra(const Graph* g, int src, int dist[N_VERTICES], int prev[N_VERTICES]);

int path_weight(const Graph* g, const int* path, int len);

int path_is_simple(const int* path, int len);

#endif
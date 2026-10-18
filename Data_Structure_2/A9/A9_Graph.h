#ifndef A9_GRAPH_H
#define A9_GRAPH_H

#ifndef N_VERTICES
#define N_VERTICES 10
#endif
#ifndef N_EDGES
#define N_EDGES    20
#endif
#define WEIGHT_MIN 1
#define WEIGHT_MAX 100

typedef struct { int u, v, w; } Edge;

typedef struct {
    int  n;
    int  m;
    Edge edges[N_EDGES];
    int  w[N_VERTICES][N_VERTICES];
} Graph;

void graph_generate(Graph* g);

int graph_check_vertex_count(const Graph* g);
int graph_check_edge_count(const Graph* g);
int graph_check_weights(const Graph* g);
int graph_check_simple(const Graph* g);
int graph_check_connected(const Graph* g);

#endif
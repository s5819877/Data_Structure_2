#include <stdlib.h>
#include <string.h>
#include "A9_Graph.h"

static int cmp_edge(const void* a, const void* b)
{
    const Edge* x = (const Edge*)a, * y = (const Edge*)b;
    if (x->u != y->u) return x->u - y->u;
    return x->v - y->v;
}

void graph_generate(Graph* g)
{
    int n = N_VERTICES;
    int perm[N_VERTICES];
    unsigned char used[N_VERTICES][N_VERTICES];

    memset(g, 0, sizeof(*g));
    memset(used, 0, sizeof(used));
    g->n = n;
    g->m = 0;

    for (int i = 0; i < n; i++) perm[i] = i;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }

    for (int i = 1; i < n; i++) {
        int a = perm[i], b = perm[rand() % i];
        used[a][b] = used[b][a] = 1;
        g->edges[g->m].u = a < b ? a : b;
        g->edges[g->m].v = a < b ? b : a;
        g->m++;
    }

    while (g->m < N_EDGES) {
        int a = rand() % n, b = rand() % n;
        if (a == b || used[a][b]) continue;
        used[a][b] = used[b][a] = 1;
        g->edges[g->m].u = a < b ? a : b;
        g->edges[g->m].v = a < b ? b : a;
        g->m++;
    }

    for (int i = 0; i < g->m; i++)
        g->edges[i].w = WEIGHT_MIN + rand() % (WEIGHT_MAX - WEIGHT_MIN + 1);
    qsort(g->edges, (size_t)g->m, sizeof(Edge), cmp_edge);
    for (int i = 0; i < g->m; i++) {
        g->w[g->edges[i].u][g->edges[i].v] = g->edges[i].w;
        g->w[g->edges[i].v][g->edges[i].u] = g->edges[i].w;
    }
}

int graph_check_vertex_count(const Graph* g)
{
    if (g->n != N_VERTICES) return 0;
    for (int i = 0; i < g->m; i++)
        if (g->edges[i].u < 0 || g->edges[i].u >= g->n ||
            g->edges[i].v < 0 || g->edges[i].v >= g->n) return 0;
    return 1;
}

int graph_check_edge_count(const Graph* g)
{
    int cells = 0;
    for (int i = 0; i < g->n; i++)
        for (int j = i + 1; j < g->n; j++) if (g->w[i][j]) cells++;
    return g->m == N_EDGES && cells == N_EDGES;
}

int graph_check_weights(const Graph* g)
{
    for (int i = 0; i < g->m; i++) {
        const Edge* e = &g->edges[i];
        if (e->w < WEIGHT_MIN || e->w > WEIGHT_MAX) return 0;
        if (g->w[e->u][e->v] != e->w || g->w[e->v][e->u] != e->w) return 0;
    }
    return 1;
}

int graph_check_simple(const Graph* g)
{
    unsigned char seen[N_VERTICES][N_VERTICES];
    memset(seen, 0, sizeof(seen));
    for (int i = 0; i < g->m; i++) {
        int u = g->edges[i].u, v = g->edges[i].v;
        if (u == v) return 0; 
        if (seen[u][v] || seen[v][u]) return 0; 
        seen[u][v] = seen[v][u] = 1;
    }
    for (int i = 0; i < g->n; i++)
        if (g->w[i][i] != 0) return 0;
    return 1;
}

int graph_check_connected(const Graph* g)
{
    int queue[N_VERTICES], front = 0, rear = 0, visited = 1;
    unsigned char seen[N_VERTICES];
    memset(seen, 0, sizeof(seen));
    queue[rear++] = 0;
    seen[0] = 1;
    while (front < rear) {
        int u = queue[front++];
        for (int v = 0; v < g->n; v++)
            if (g->w[u][v] && !seen[v]) { seen[v] = 1; queue[rear++] = v; visited++; }
    }
    return visited == g->n;
}
#include <string.h>
#include "A9_Shortest.h"

void floyd_warshall(const Graph* g, AllPairs* ap)
{
    int n = g->n;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) { ap->dist[i][j] = 0;         ap->next[i][j] = j; }
            else if (g->w[i][j]) { ap->dist[i][j] = g->w[i][j]; ap->next[i][j] = j; }
            else { ap->dist[i][j] = INF;       ap->next[i][j] = -1; }
        }
    }

    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++) {
            if (ap->dist[i][k] >= INF) continue;
            for (int j = 0; j < n; j++) {
                if (ap->dist[k][j] >= INF) continue;
                int through_k = ap->dist[i][k] + ap->dist[k][j];
                if (through_k < ap->dist[i][j]) {
                    ap->dist[i][j] = through_k;
                    ap->next[i][j] = ap->next[i][k];
                }
            }
        }
}

int path_from_next(const AllPairs* ap, int u, int v, int* path)
{
    if (ap->next[u][v] < 0) return 0;
    int len = 0;
    path[len++] = u;
    while (u != v) {
        u = ap->next[u][v];
        path[len++] = u;
        if (len > N_VERTICES) return 0;
    }
    return len;
}

void dijkstra(const Graph* g, int src, int dist[N_VERTICES], int prev[N_VERTICES])
{
    int n = g->n;
    unsigned char done[N_VERTICES];

    for (int i = 0; i < n; i++) { dist[i] = INF; prev[i] = -1; done[i] = 0; }
    dist[src] = 0;

    for (int iter = 0; iter < n; iter++) {
        int u = -1;
        for (int v = 0; v < n; v++)
            if (!done[v] && dist[v] < INF && (u < 0 || dist[v] < dist[u])) u = v;
        if (u < 0) break;
        done[u] = 1;
        for (int v = 0; v < n; v++)
            if (g->w[u][v] && !done[v] && dist[u] + g->w[u][v] < dist[v]) {
                dist[v] = dist[u] + g->w[u][v];
                prev[v] = u;
            }
    }
}

int path_weight(const Graph* g, const int* path, int len)
{
    int sum = 0;
    for (int i = 0; i + 1 < len; i++) {
        int w = g->w[path[i]][path[i + 1]];
        if (w == 0) return -1;
        sum += w;
    }
    return sum;
}

int path_is_simple(const int* path, int len)
{
    for (int i = 0; i < len; i++)
        for (int j = i + 1; j < len; j++)
            if (path[i] == path[j]) return 0;
    return 1;
}
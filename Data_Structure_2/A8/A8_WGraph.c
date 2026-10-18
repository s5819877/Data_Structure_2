#include <stdio.h>
#include <stdlib.h>
#include "A8_WGraph.h"

int generate_connected_graph(Edge* edges, int n, int E, int wmax)
{
    if (n < 1 || wmax < 1 || E < n - 1 || (long)E >(long)n * (n - 1) / 2) return -1;

    unsigned char* used = (unsigned char*)calloc((size_t)n * (size_t)n, 1);
    int* perm = (int*)malloc((size_t)n * sizeof(int));
    if (!used || !perm) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }

    for (int i = 0; i < n; i++) perm[i] = i;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }

    int cnt = 0;

    for (int i = 1; i < n; i++) {
        int u = perm[i], v = perm[rand() % i];
        used[u * n + v] = used[v * n + u] = 1;
        edges[cnt].u = u; edges[cnt].v = v; edges[cnt].w = 1 + rand() % wmax;
        cnt++;
    }

    while (cnt < E) {
        int u = rand() % n, v = rand() % n;
        if (u == v || used[u * n + v]) continue;
        used[u * n + v] = used[v * n + u] = 1;
        edges[cnt].u = u; edges[cnt].v = v; edges[cnt].w = 1 + rand() % wmax;
        cnt++;
    }

    for (int i = E - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Edge t = edges[i]; edges[i] = edges[j]; edges[j] = t;
    }

    free(used);
    free(perm);
    return 0;
}
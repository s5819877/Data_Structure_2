#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "A8_Prim.h"

int prim_mst(const MatrixGraph* g, int start, Edge* out, MstStats* st)
{
    int n = g->n;

    memset(st, 0, sizeof(*st));
    if (n <= 1) return 0;

    /* 보조 자료구조 */
    int* key = (int*)malloc((size_t)n * sizeof(int));
    int* parent = (int*)malloc((size_t)n * sizeof(int));
    unsigned char* in_mst = (unsigned char*)malloc((size_t)n);
    if (!key || !parent || !in_mst) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }

    mem_reset(&st->aux);
    mem_add(&st->aux, "key[] (최소 연결 비용)", (size_t)n * sizeof(int));
    mem_add(&st->aux, "parent[] (연결된 MST 정점)", (size_t)n * sizeof(int));
    mem_add(&st->aux, "in_mst[] (방문 여부)", (size_t)n * sizeof(unsigned char));
    mem_add(&st->aux, "MST 결과 간선 배열", (size_t)(n - 1) * sizeof(Edge));

    for (int i = 0; i < n; i++) { key[i] = INT_MAX; parent[i] = -1; in_mst[i] = 0; }
    key[start] = 0;

    int taken = 0;
    for (int iter = 0; iter < n; iter++) {
        int u = -1, best = INT_MAX;
        for (int v = 0; v < n; v++) {
            st->ops_select++;
            if (!in_mst[v] && key[v] < best) { best = key[v]; u = v; }
        }
        if (u < 0) break;

        in_mst[u] = 1;
        if (parent[u] >= 0) { 
            out[taken].u = parent[u];
            out[taken].v = u;
            out[taken].w = key[u];
            taken++;
            st->cost += key[u];
        }

        const int* row = &g->w[u * n];
        for (int v = 0; v < n; v++) {
            st->ops_cells++;
            int w = row[v];
            if (w != 0 && !in_mst[v] && w < key[v]) { key[v] = w; parent[v] = u; }
        }
    }

    free(key);
    free(parent);
    free(in_mst);

    st->edge_count = taken;
    return taken;
}
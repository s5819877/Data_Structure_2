#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "A8_Kruskal.h"

static void merge_sort_edges(Edge* a, Edge* tmp, int m, long* cmp)
{
    Edge* src = a, * dst = tmp;

    for (int width = 1; width < m; width *= 2) {
        for (int lo = 0; lo < m; lo += 2 * width) {
            int mid = lo + width < m ? lo + width : m;
            int hi = lo + 2 * width < m ? lo + 2 * width : m;
            int i = lo, j = mid, k = lo;

            while (i < mid && j < hi) {
                (*cmp)++; 
                if (src[i].w <= src[j].w) dst[k++] = src[i++];
                else                      dst[k++] = src[j++];
            }
            while (i < mid) dst[k++] = src[i++];
            while (j < hi)  dst[k++] = src[j++];
        }
        Edge* t = src; src = dst; dst = t;
    }
    if (src != a) memcpy(a, src, (size_t)m * sizeof(Edge));
}

static int uf_find(int* parent, int x)
{
    int root = x;
    while (parent[root] != root) root = parent[root];
    while (parent[x] != root) { 
        int nx = parent[x];
        parent[x] = root;
        x = nx;
    }
    return root;
}

static int uf_union(int* parent, int* rank, int a, int b)
{
    int ra = uf_find(parent, a), rb = uf_find(parent, b);
    if (ra == rb) return 0;
    if (rank[ra] < rank[rb]) { int t = ra; ra = rb; rb = t; }
    parent[rb] = ra;
    if (rank[ra] == rank[rb]) rank[ra]++;
    return 1;
}

int kruskal_mst(const ListGraph* g, Edge* out, MstStats* st)
{
    int n = g->n;
    int m = g->edge_count;

    memset(st, 0, sizeof(*st));
    if (n <= 1) return 0;

    /* 보조 자료구조 */
    Edge* edges = (Edge*)malloc((size_t)m * sizeof(Edge));
    Edge* tmp = (Edge*)malloc((size_t)m * sizeof(Edge));
    int* parent = (int*)malloc((size_t)n * sizeof(int));
    int* rank = (int*)malloc((size_t)n * sizeof(int));
    if (!edges || !tmp || !parent || !rank) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }

    mem_reset(&st->aux);
    mem_add(&st->aux, "간선 배열 (리스트에서 모은 복사본)", (size_t)m * sizeof(Edge));
    mem_add(&st->aux, "정렬 보조 버퍼", (size_t)m * sizeof(Edge));
    mem_add(&st->aux, "Union-Find (parent + rank)", (size_t)2 * (size_t)n * sizeof(int));
    mem_add(&st->aux, "MST 결과 간선 배열", (size_t)(n - 1) * sizeof(Edge));

    int k = 0;
    for (int u = 0; u < n; u++) {
        for (const WNode* p = g->head[u]; p; p = p->next) {
            st->ops_collect++;
            if (u < p->vertex) {
                edges[k].u = u; edges[k].v = p->vertex; edges[k].w = p->weight;
                k++;
            }
        }
    }

    merge_sort_edges(edges, tmp, k, &st->ops_sort);

    for (int i = 0; i < n; i++) { parent[i] = i; rank[i] = 0; }

    int taken = 0;
    for (int i = 0; i < k && taken < n - 1; i++) {
        st->ops_examined++;
        if (uf_union(parent, rank, edges[i].u, edges[i].v)) {
            out[taken++] = edges[i];
            st->cost += edges[i].w;
        }
    }

    free(edges);
    free(tmp);
    free(parent);
    free(rank);

    st->edge_count = taken;
    return taken;
}
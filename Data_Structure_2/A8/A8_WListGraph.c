#include <stdio.h>
#include <stdlib.h>
#include "A8_WListGraph.h"

ListGraph* wl_create(int n)
{
    ListGraph* g = (ListGraph*)malloc(sizeof(ListGraph));
    if (!g) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    g->head = (WNode**)calloc((size_t)n, sizeof(WNode*));
    if (!g->head) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    g->n = n;
    g->edge_count = 0;
    return g;
}

void wl_destroy(ListGraph* g)
{
    if (!g) return;
    for (int u = 0; u < g->n; u++) {
        WNode* p = g->head[u];
        while (p) { WNode* nx = p->next; free(p); p = nx; }
    }
    free(g->head);
    free(g);
}

static void push_front(ListGraph* g, int u, int v, int w)
{
    WNode* p = (WNode*)malloc(sizeof(WNode));
    if (!p) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    p->vertex = v;
    p->weight = w;
    p->next = g->head[u];
    g->head[u] = p;
}

void wl_add_edge(ListGraph* g, int u, int v, int w)
{
    push_front(g, u, v, w);
    push_front(g, v, u, w);
    g->edge_count++;
}

void wl_memory(const ListGraph* g, MemReport* m)
{
    mem_reset(m);
    mem_add(m, "그래프 구조체", sizeof(ListGraph));
    mem_add(m, "정점별 리스트 헤드 배열", (size_t)g->n * sizeof(WNode*));
    mem_add(m, "간선 노드 (간선당 2개)", (size_t)2 * (size_t)g->edge_count * sizeof(WNode));
}

int wl_is_connected(const ListGraph* g)
{
    int n = g->n;
    if (n == 0) return 1;
    int* queue = (int*)malloc((size_t)n * sizeof(int));
    unsigned char* seen = (unsigned char*)calloc((size_t)n, 1);
    int front = 0, rear = 0, visited = 1;
    queue[rear++] = 0;
    seen[0] = 1;
    while (front < rear) {
        int u = queue[front++];
        for (const WNode* p = g->head[u]; p; p = p->next) {
            if (!seen[p->vertex]) {
                seen[p->vertex] = 1;
                queue[rear++] = p->vertex;
                visited++;
            }
        }
    }
    free(queue);
    free(seen);
    return visited == n;
}
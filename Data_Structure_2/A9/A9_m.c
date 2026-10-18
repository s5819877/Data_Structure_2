#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "A9_Graph.h"
#include "A9_Shortest.h"

static void print_path(const int* path, int len)
{
    for (int i = 0; i < len; i++) printf(i ? " -> %d" : "%d", path[i]);
}

static int dijkstra_path_weight(const Graph* g, const int prev[N_VERTICES], int s, int t)
{
    int rev[N_VERTICES], len = 0;
    for (int x = t; x != -1; x = prev[x]) {
        if (len >= N_VERTICES) return -1;
        rev[len++] = x;
    }
    if (rev[len - 1] != s) return -1;
    int fwd[N_VERTICES];
    for (int i = 0; i < len; i++) fwd[i] = rev[len - 1 - i];
    return path_weight(g, fwd, len);
}

static const char* yes_no(int ok) { return ok ? "예" : "아니오"; }

int main(int argc, char* argv[])
{
    unsigned seed = (argc >= 2) ? (unsigned)atoi(argv[1]) : (unsigned)time(NULL);
    srand(seed);
    printf("난수 시드 : %u  (재현하려면 './shortest_paths %u' 로 실행)\n\n", seed, seed);

    Graph g;
    graph_generate(&g);

    printf("Graph\n");
    printf("Vertices : %d\n", g.n);
    printf("Edges    : %d\n\n", g.m);
    printf("Edges\n");
    for (int i = 0; i < g.m; i++)
        printf("(%d, %d) Weight: %d\n", g.edges[i].u, g.edges[i].v, g.edges[i].w);

    AllPairs ap;
    floyd_warshall(&g, &ap);

    int dj_dist[N_VERTICES][N_VERTICES], dj_prev[N_VERTICES][N_VERTICES];
    for (int s = 0; s < g.n; s++) dijkstra(&g, s, dj_dist[s], dj_prev[s]);

    printf("\nShortest Paths\n");

    int pairs = 0;
    int all_paths_ok = 1;
    int all_sums_ok = 1;
    int all_simple_ok = 1;
    int all_dijkstra_ok = 1;

    for (int u = 0; u < g.n; u++) {
        for (int v = u + 1; v < g.n; v++) {
            int path[N_VERTICES + 1];
            int len = path_from_next(&ap, u, v, path);
            int dist = ap.dist[u][v];

            printf("\n%d -> %d\n", u, v);
            printf("Path     : ");
            if (len > 0) print_path(path, len); else printf("(경로 없음)");
            printf("\nDistance : %d\n", dist);
            pairs++;

            if (len < 2 || path[0] != u || path[len - 1] != v) all_paths_ok = 0;
            else {
                if (path_weight(&g, path, len) != dist) all_sums_ok = 0;
                if (!path_is_simple(path, len)) all_simple_ok = 0;
            }
            if (dj_dist[u][v] != dist || dj_dist[v][u] != dist ||
                dijkstra_path_weight(&g, dj_prev[u], u, v) != dist) all_dijkstra_ok = 0;
        }
    }
    printf("\nTotal vertex pairs : %d\n", pairs);

    printf("\n========== 확인 사항 ==========\n");
    printf(" 1. 정점이 정확히 %d개인가?                          : %s\n", N_VERTICES, yes_no(graph_check_vertex_count(&g)));
    printf(" 2. 간선이 정확히 %d개인가?                          : %s\n", N_EDGES, yes_no(graph_check_edge_count(&g)));
    printf(" 3. 각 간선에 1~%d 의 가중치가 올바르게 부여되었는가?  : %s\n", WEIGHT_MAX, yes_no(graph_check_weights(&g)));
    printf(" 4. 자기 자신 연결이나 중복 간선이 없는가?            : %s\n", yes_no(graph_check_simple(&g)));
    printf(" 5. 생성된 그래프가 연결 그래프인가?                  : %s\n", yes_no(graph_check_connected(&g)));
    printf(" 6. 서로 다른 정점 쌍 %d개의 결과가 모두 출력되었는가? : %s  (출력한 쌍 %d개)\n",
        N_VERTICES * (N_VERTICES - 1) / 2, yes_no(pairs == N_VERTICES * (N_VERTICES - 1) / 2), pairs);
    printf(" 7. 각 쌍에 실제 최단경로(정점 순서)가 출력되었는가?  : %s\n", yes_no(all_paths_ok && all_simple_ok));
    printf(" 8. 각 최단거리가 경로의 간선 가중치 합과 같은가?     : %s\n", yes_no(all_sums_ok));
    printf("(추가) Dijkstra 로 다시 구한 최단거리와 45쌍 모두 같은가? : %s\n", yes_no(all_dijkstra_ok));
    return 0;
}
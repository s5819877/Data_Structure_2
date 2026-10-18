#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "A8_GraphCommon.h"
#include "A8_WGraph.h"
#include "A8_WListGraph.h"
#include "A8_WMatrixGraph.h"
#include "A8_Kruskal.h"
#include "A8_Prim.h"
#include "A8_Timer.h"

#define BATCHES         31  
#define TARGET_BATCH_NS 2.0e6 

static const char* comma(unsigned long n, char* buf)
{
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lu", n);
    int oi = len + (len - 1) / 3;
    buf[oi--] = '\0';
    for (int i = len - 1, cnt = 0; i >= 0; i--) {
        buf[oi--] = tmp[i];
        if (++cnt % 3 == 0 && i != 0) buf[oi--] = ',';
    }
    return buf;
}

typedef struct {
    const char* name;
    int          V, E;
    Edge* edges;
    ListGraph* lg;
    MatrixGraph* mg;
} GraphSet;

static void build_graph_set(GraphSet* gs, const char* name, int V, int E)
{
    gs->name = name;
    gs->V = V;
    gs->E = E;
    gs->edges = (Edge*)malloc((size_t)E * sizeof(Edge));
    if (!gs->edges || generate_connected_graph(gs->edges, V, E, WEIGHT_MAX) != 0) {
        fprintf(stderr, "그래프 생성에 실패했습니다.\n");
        exit(1);
    }
    gs->lg = wl_create(V);
    gs->mg = wm_create(V);
    for (int i = 0; i < E; i++) {
        wl_add_edge(gs->lg, gs->edges[i].u, gs->edges[i].v, gs->edges[i].w);
        wm_add_edge(gs->mg, gs->edges[i].u, gs->edges[i].v, gs->edges[i].w);
    }
}

static void free_graph_set(GraphSet* gs)
{
    wl_destroy(gs->lg);
    wm_destroy(gs->mg);
    free(gs->edges);
}

static int representations_match(const GraphSet* gs)
{
    int n = gs->V;
    long nodes = 0;
    for (int u = 0; u < n; u++)
        for (const WNode* p = gs->lg->head[u]; p; p = p->next) {
            nodes++;
            if (wm_weight(gs->mg, u, p->vertex) != p->weight) return 0;
        }
    long cells = 0;
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++) if (wm_weight(gs->mg, u, v)) cells++;
    return nodes == 2L * gs->E && cells == 2L * gs->E;
}

static int is_valid_spanning_tree(const Edge* mst, int cnt, const MatrixGraph* g)
{
    int n = g->n;
    int comp[N_VERTICES];
    if (cnt != n - 1) return 0;
    for (int i = 0; i < n; i++) comp[i] = i;
    for (int i = 0; i < cnt; i++) {
        int u = mst[i].u, v = mst[i].v;
        if (mst[i].w <= 0 || wm_weight(g, u, v) != mst[i].w) return 0;
        if (comp[u] == comp[v]) return 0;
        int from = comp[v], to = comp[u];
        for (int x = 0; x < n; x++) if (comp[x] == from) comp[x] = to;
    }
    return 1;
}

static int same_edge_set(const Edge* a, const Edge* b, int cnt)
{
    for (int i = 0; i < cnt; i++) {
        int found = 0;
        for (int j = 0; j < cnt && !found; j++)
            found = (a[i].u == b[j].u && a[i].v == b[j].v) ||
            (a[i].u == b[j].v && a[i].v == b[j].u);
        if (!found) return 0;
    }
    return 1;
}

typedef struct {
    double median_ns, min_ns, max_ns;
    long   reps;
} TimeStat;

static int cmp_double(const void* a, const void* b)
{
    double x = *(const double*)a, y = *(const double*)b;
    return (x > y) - (x < y);
}

static void time_runs(void (*fn)(void*), void* ctx, TimeStat* ts)
{
    long reps = 1;
    for (;;) {
        double t0 = timer_now_ns();
        for (long r = 0; r < reps; r++) fn(ctx);
        double dt = timer_now_ns() - t0;
        if (dt >= TARGET_BATCH_NS || reps >= (1L << 24)) break;
        reps *= 2;
    }

    double samples[BATCHES];
    for (int b = 0; b < BATCHES; b++) {
        double t0 = timer_now_ns();
        for (long r = 0; r < reps; r++) fn(ctx);
        samples[b] = (timer_now_ns() - t0) / (double)reps;
    }
    qsort(samples, BATCHES, sizeof(double), cmp_double);
    ts->median_ns = samples[BATCHES / 2];
    ts->min_ns = samples[0];
    ts->max_ns = samples[BATCHES - 1];
    ts->reps = reps;
}

typedef struct { const ListGraph* g; Edge out[N_VERTICES]; MstStats st; } KruskalCtx;
typedef struct { const MatrixGraph* g; Edge out[N_VERTICES]; MstStats st; } PrimCtx;

static void run_kruskal(void* p) { KruskalCtx* c = (KruskalCtx*)p; kruskal_mst(c->g, c->out, &c->st); }
static void run_prim(void* p) { PrimCtx* c = (PrimCtx*)p;    prim_mst(c->g, 0, c->out, &c->st); }

typedef struct {
    int       no;
    char      title[96];
    int       V, E;
    int       mst_edges;
    long      cost;
    MemReport graph_mem, aux_mem;
    size_t    total_mem;
    TimeStat  time;
    Edge      mst[N_VERTICES];
    MstStats  st;
    int       is_kruskal;
    int       valid;
} CaseResult;

static void run_case_kruskal(CaseResult* r, int no, const char* title, const GraphSet* gs)
{
    static KruskalCtx ctx;
    memset(r, 0, sizeof(*r));
    ctx.g = gs->lg;
    run_kruskal(&ctx);

    r->no = no;  snprintf(r->title, sizeof(r->title), "%s", title);
    r->V = gs->V;  r->E = gs->E;  r->is_kruskal = 1;
    r->mst_edges = ctx.st.edge_count;  r->cost = ctx.st.cost;
    memcpy(r->mst, ctx.out, sizeof(r->mst));
    r->st = ctx.st;
    wl_memory(gs->lg, &r->graph_mem);
    r->aux_mem = ctx.st.aux;
    r->total_mem = r->graph_mem.total + r->aux_mem.total;
    r->valid = is_valid_spanning_tree(r->mst, r->mst_edges, gs->mg);

    time_runs(run_kruskal, &ctx, &r->time);
}

static void run_case_prim(CaseResult* r, int no, const char* title, const GraphSet* gs)
{
    static PrimCtx ctx;
    memset(r, 0, sizeof(*r));
    ctx.g = gs->mg;
    run_prim(&ctx);

    r->no = no;  snprintf(r->title, sizeof(r->title), "%s", title);
    r->V = gs->V;  r->E = gs->E;  r->is_kruskal = 0;
    r->mst_edges = ctx.st.edge_count;  r->cost = ctx.st.cost;
    memcpy(r->mst, ctx.out, sizeof(r->mst));
    r->st = ctx.st;
    wm_memory(gs->mg, &r->graph_mem);
    r->aux_mem = ctx.st.aux;
    r->total_mem = r->graph_mem.total + r->aux_mem.total;
    r->valid = is_valid_spanning_tree(r->mst, r->mst_edges, gs->mg);

    time_runs(run_prim, &ctx, &r->time);
}

static void print_graph_info(const GraphSet* gs)
{
    int n = gs->V;
    double max_edges = (double)n * (n - 1) / 2.0;
    int wmin = WEIGHT_MAX + 1, wmax = 0;
    long wsum = 0;
    for (int i = 0; i < gs->E; i++) {
        if (gs->edges[i].w < wmin) wmin = gs->edges[i].w;
        if (gs->edges[i].w > wmax) wmax = gs->edges[i].w;
        wsum += gs->edges[i].w;
    }
    printf("[%s]\n", gs->name);
    printf("  Vertices : %d\n", n);
    printf("  Edges    : %d  (가능한 최대 간선 %.0f개 중 %.1f%%, 평균 차수 %.2f)\n",
        gs->E, max_edges, 100.0 * gs->E / max_edges, 2.0 * gs->E / n);
    printf("  가중치   : 1 ~ %d 범위에서 무작위 (실제 최소 %d / 최대 %d / 평균 %.1f)\n",
        WEIGHT_MAX, wmin, wmax, (double)wsum / gs->E);
    printf("  (검증) 연결 그래프인가 (BFS) : %s\n", wl_is_connected(gs->lg) ? "예" : "아니오!!");
    printf("  (검증) 인접 리스트와 인접 행렬이 같은 그래프인가 : %s\n",
        representations_match(gs) ? "일치" : "불일치!!");
}

static void print_case(const CaseResult* r)
{
    char b[32];
    printf("Case %d : %s\n", r->no, r->title);
    printf("Vertices      : %d\n", r->V);
    printf("Edges         : %d\n", r->E);
    printf("MST Edges     : %d\n", r->mst_edges);
    printf("MST Cost      : %ld\n", r->cost);
    printf("Memory        : %s Bytes\n", comma(r->total_mem, b));
    printf("Execution Time: %.5f ms\n\n", r->time.median_ns / 1e6);
}

static void print_mst_edges(const CaseResult* r)
{
    printf("Case %d : %s  (간선 %d개, 총 비용 %ld)\n", r->no, r->title, r->mst_edges, r->cost);
    for (int i = 0; i < r->mst_edges; i++) {
        int u = r->mst[i].u < r->mst[i].v ? r->mst[i].u : r->mst[i].v;
        int v = r->mst[i].u < r->mst[i].v ? r->mst[i].v : r->mst[i].u;
        if (i % 6 == 0) printf("  ");
        printf("(%2d,%2d):%-3d  ", u, v, r->mst[i].w);
        if (i % 6 == 5 || i == r->mst_edges - 1) printf("\n");
    }
    printf("\n");
}

static void print_memory_detail(const CaseResult* r)
{
    char b[32];
    printf("Case %d : %s   합계 %s Bytes\n", r->no, r->title, comma(r->total_mem, b));
    printf("  [그래프 표현] %s Bytes\n", comma(r->graph_mem.total, b));
    for (int i = 0; i < r->graph_mem.count; i++)
        printf("    %10s  %s\n", comma(r->graph_mem.item[i].bytes, b), r->graph_mem.item[i].name);
    printf("  [MST 알고리즘 보조 자료구조] %s Bytes\n", comma(r->aux_mem.total, b));
    for (int i = 0; i < r->aux_mem.count; i++)
        printf("    %10s  %s\n", comma(r->aux_mem.item[i].bytes, b), r->aux_mem.item[i].name);
    printf("\n");
}

int main(int argc, char* argv[])
{
    unsigned seed = (argc >= 2) ? (unsigned)atoi(argv[1]) : (unsigned)time(NULL);
    srand(seed);
    printf("난수 시드 : %u  (재현하려면 './mst_compare %u' 로 실행)\n\n", seed, seed);

    GraphSet sparse, dense;
    build_graph_set(&sparse, "Sparse Graph", N_VERTICES, SPARSE_EDGES);
    build_graph_set(&dense, "Dense Graph", N_VERTICES, DENSE_EDGES);

    printf("===== 1. 생성된 그래프 =====\n");
    print_graph_info(&sparse);
    printf("\n");
    print_graph_info(&dense);

    static CaseResult c[4];
    run_case_kruskal(&c[0], 1, "Sparse Graph - Adjacency List - Kruskal", &sparse);
    run_case_kruskal(&c[1], 2, "Dense Graph - Adjacency List - Kruskal", &dense);
    run_case_prim(&c[2], 3, "Sparse Graph - Adjacency Matrix - Prim", &sparse);
    run_case_prim(&c[3], 4, "Dense Graph - Adjacency Matrix - Prim", &dense);

    printf("\n===== 2. 측정 결과 (과제 출력 형식) =====\n\n");
    for (int i = 0; i < 4; i++) print_case(&c[i]);

    printf("===== 3. MST 에 포함된 간선 (알고리즘이 고른 순서, 간선 표기 : (정점,정점):가중치) =====\n\n");
    for (int i = 0; i < 4; i++) print_mst_edges(&c[i]);

    printf("===== 4. MST 결과 검증 =====\n");
    for (int i = 0; i < 4; i++)
        printf("  Case %d : 간선 %d개 (정점 %d개 -> %d개여야 함), 신장 트리 조건(사이클 없음, 원래 그래프의 간선) %s\n",
            c[i].no, c[i].mst_edges, c[i].V, c[i].V - 1, c[i].valid ? "만족" : "위반!!");
    printf("  희소 그래프 : Kruskal 비용 %ld / Prim 비용 %ld -> %s,  간선 구성 %s\n",
        c[0].cost, c[2].cost, c[0].cost == c[2].cost ? "비용 동일" : "비용 다름!!",
        same_edge_set(c[0].mst, c[2].mst, c[0].mst_edges) ? "완전히 같음" : "일부 다름 (동점 가중치 때문, 비용은 같음)");
    printf("  밀집 그래프 : Kruskal 비용 %ld / Prim 비용 %ld -> %s,  간선 구성 %s\n\n",
        c[1].cost, c[3].cost, c[1].cost == c[3].cost ? "비용 동일" : "비용 다름!!",
        same_edge_set(c[1].mst, c[3].mst, c[1].mst_edges) ? "완전히 같음" : "일부 다름 (동점 가중치 때문, 비용은 같음)");

    printf("===== 5. 메모리 사용량 상세 (sizeof 기준, malloc 관리 영역 제외) =====\n\n");
    for (int i = 0; i < 4; i++) print_memory_detail(&c[i]);

    printf("===== 6. 실행시간 상세 =====\n");
    printf("(측정 구간 : MST 알고리즘 시작 ~ 끝. 알고리즘 안에서 하는 간선 수집·정렬·보조 배열 할당/해제를 포함하고,\n");
    printf(" 그래프 생성과 출력은 제외. 같은 계산을 반복하는 배치 %d개를 재고, 배치별 1회 평균의 중앙값을 사용)\n", BATCHES);
    printf("%-6s %-42s %12s %12s %12s %12s\n", "Case", "", "median(us)", "min(us)", "max(us)", "reps/batch");
    for (int i = 0; i < 4; i++)
        printf("%-6d %-42s %12.3f %12.3f %12.3f %12ld\n", c[i].no, c[i].title,
            c[i].time.median_ns / 1e3, c[i].time.min_ns / 1e3, c[i].time.max_ns / 1e3, c[i].time.reps);

    printf("\n===== 7. 연산 횟수 (환경과 무관하게 재현되는, 시간 차이의 원인을 설명하는 값) =====\n");
    printf("[Kruskal]  nodes visited = 간선을 모으려고 방문한 리스트 노드 수 / sort comparisons = 정렬 중 가중치 비교 횟수\n");
    printf("           edges examined = 사이클 검사를 한 간선 수 / 전체 간선 수\n");
    printf("%-6s %-42s %14s %14s %20s\n", "Case", "", "nodes visited", "sort cmps", "edges examined");
    for (int i = 0; i < 2; i++)
        printf("%-6d %-42s %14ld %14ld %12ld / %-5d (%.1f%%)\n", c[i].no, c[i].title,
            c[i].st.ops_collect, c[i].st.ops_sort, c[i].st.ops_examined, c[i].E,
            100.0 * c[i].st.ops_examined / c[i].E);
    printf("[Prim]  key[] checks = 최소 key 정점을 고르려고 key[]를 확인한 횟수 / matrix cells = 인접 행렬 칸을 읽은 횟수\n");
    printf("%-6s %-42s %16s %16s\n", "Case", "", "key[] checks", "matrix cells");
    for (int i = 2; i < 4; i++)
        printf("%-6d %-42s %16ld %16ld\n", c[i].no, c[i].title, c[i].st.ops_select, c[i].st.ops_cells);

    free_graph_set(&sparse);
    free_graph_set(&dense);
    return 0;
}
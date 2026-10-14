#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "A7_GraphCommon.h"
#include "A7_MatrixGraph.h"
#include "A7_ListGraph.h"

#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 33))
#include <malloc.h>
#define HAVE_MALLINFO2 1

static size_t heap_in_use(void)
{
    struct mallinfo2 mi = mallinfo2();
    return (size_t)mi.uordblks + (size_t)mi.hblkhd;
}
#else
#define HAVE_MALLINFO2 0
static size_t heap_in_use(void) { return 0; }
#endif

#define SPARSE_EDGES 100
#define DENSE_EDGES  4000
#define TRIALS       1000 

typedef struct { int u, v; } Edge;

static const char* comma(unsigned long n, char* buf)
{
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lu", n);
    int out = len + (len - 1) / 3;
    int oi = out;
    buf[oi--] = '\0';
    for (int i = len - 1, cnt = 0; i >= 0; i--) {
        buf[oi--] = tmp[i];
        if (++cnt % 3 == 0 && i != 0) buf[oi--] = ',';
    }
    return buf;
}

static int rnd(int n) { return rand() % n; }

static void generate_edges(Edge* edges, int E, int n)
{
    static unsigned char used[N_VERTICES][N_VERTICES];
    memset(used, 0, sizeof(used));

    int cnt = 0;
    while (cnt < E) {
        int u = rnd(n), v = rnd(n);
        if (u == v || used[u][v]) continue;
        used[u][v] = used[v][u] = 1;
        edges[cnt].u = u;
        edges[cnt].v = v;
        cnt++;
    }
}

typedef struct {
    const char* name;
    int          E;
    Edge* edges;
    MatrixGraph* mg;
    ListGraph* lg;
    MemBreakdown mm, ml;
    size_t       real_m, real_l;
} GraphSet;

static void build_graph_set(GraphSet* gs, const char* name, int E, int n)
{
    gs->name = name;
    gs->E = E;
    gs->edges = (Edge*)malloc((size_t)E * sizeof(Edge));
    generate_edges(gs->edges, E, n);

    size_t b0 = heap_in_use();
    gs->mg = mg_create(n);
    for (int i = 0; i < E; i++) mg_build_edge(gs->mg, gs->edges[i].u, gs->edges[i].v);
    size_t b1 = heap_in_use();
    gs->lg = lg_create(n);
    for (int i = 0; i < E; i++) lg_build_edge(gs->lg, gs->edges[i].u, gs->edges[i].v);
    size_t b2 = heap_in_use();

    gs->real_m = b1 - b0;
    gs->real_l = b2 - b1;
    mg_memory(gs->mg, &gs->mm);
    lg_memory(gs->lg, &gs->ml);
}

static void free_graph_set(GraphSet* gs)
{
    mg_destroy(gs->mg);
    lg_destroy(gs->lg);
    free(gs->edges);
}

static int cmp_int(const void* a, const void* b)
{
    return *(const int*)a - *(const int*)b;
}

static int graphs_equal(const GraphSet* gs)
{
    int a[N_VERTICES], b[N_VERTICES];
    for (int u = 0; u < N_VERTICES; u++) {
        long d1 = 0, d2 = 0;
        int ka = mg_neighbors(gs->mg, u, a, &d1);
        int kb = lg_neighbors(gs->lg, u, b, &d2);
        if (ka != kb) return 0;
        qsort(a, (size_t)ka, sizeof(int), cmp_int);
        qsort(b, (size_t)kb, sizeof(int), cmp_int);
        if (memcmp(a, b, (size_t)ka * sizeof(int)) != 0) return 0;
    }
    return gs->mg->edge_count == gs->lg->edge_count;
}

static int memory_consistent(const GraphSet* gs)
{
    int n = N_VERTICES;
    long ones = 0, nodes = 0;
    for (int u = 0; u < n; u++) {
        nodes += lg_degree(gs->lg, u);
        for (int v = 0; v < n; v++) ones += mg_peek(gs->mg, u, v);
    }
    return ones == 2L * gs->mg->edge_count &&
        nodes == (long)gs->lg->node_count &&
        nodes == 2L * gs->lg->edge_count;
}

static void print_graph_info(const GraphSet* gs)
{
    int n = N_VERTICES;
    int mind = n, maxd = 0, isolated = 0;
    for (int u = 0; u < n; u++) {
        int d = lg_degree(gs->lg, u);
        if (d < mind) mind = d;
        if (d > maxd) maxd = d;
        if (d == 0) isolated++;
    }
    double max_edges = (double)n * (n - 1) / 2.0;

    printf("[%s]\n", gs->name);
    printf("  Vertices : %d\n", n);
    printf("  Edges    : %d  (가능한 최대 간선 %.0f개 중 %.1f%%)\n",
        gs->E, max_edges, 100.0 * gs->E / max_edges);
    printf("  정점 차수 : 평균 %.2f / 최소 %d / 최대 %d / 차수 0인 정점 %d개\n",
        2.0 * gs->E / n, mind, maxd, isolated);
    printf("  행렬에서 1인 칸 : %d / %d (%.1f%%)  <- 나머지는 모두 0으로 낭비되는 칸\n",
        2 * gs->E, n * n, 100.0 * 2 * gs->E / (n * n));
    printf("  (검증) 인접 행렬과 인접 리스트가 같은 그래프인지 : %s\n",
        graphs_equal(gs) ? "일치" : "불일치!!");
}

static void print_vertices(const char* label, const int* a, int k)
{
    printf("    %-18s (%d개) :", label, k);
    if (k == 0) { printf(" 없음\n"); return; }
    for (int i = 0; i < k; i++) {
        if (i % 20 == 0 && i != 0) printf("\n%*s", 4 + 18 + 1 + 6 + 3 + 1, "");
        printf(" %d", a[i]);
    }
    printf("\n");
}



typedef struct {
    int  iu, iv;
    int  du, dv;
    int  cu, cv;
    int  connected;
    int  au;
    long ins_m, ins_l, del_m, del_l, conn_m, conn_l, adj_m, adj_l;
    int  nb_m[N_VERTICES], nb_l[N_VERTICES];
    int  nb_cnt_m, nb_cnt_l;
} SingleRun;

static void run_single(GraphSet* gs, SingleRun* r)
{
    MatrixGraph* mg = gs->mg;
    ListGraph* lg = gs->lg;
    int n = N_VERTICES;
    memset(r, 0, sizeof(*r));

    do { r->iu = rnd(n); r->iv = rnd(n); } while (r->iu == r->iv || mg_peek(mg, r->iu, r->iv));
    int okm = mg_insert_edge(mg, r->iu, r->iv, &r->ins_m);
    int okl = lg_insert_edge(lg, r->iu, r->iv, &r->ins_l);
    if (!okm || !okl) fprintf(stderr, "[경고] 삽입 결과 불일치 (matrix=%d list=%d)\n", okm, okl);

    do { r->du = rnd(n); r->dv = rnd(n); } while (r->du == r->dv || !mg_peek(mg, r->du, r->dv) ||
        (r->du == r->iu && r->dv == r->iv) || (r->du == r->iv && r->dv == r->iu));
    okm = mg_delete_edge(mg, r->du, r->dv, &r->del_m);
    okl = lg_delete_edge(lg, r->du, r->dv, &r->del_l);
    if (!okm || !okl) fprintf(stderr, "[경고] 삭제 결과 불일치 (matrix=%d list=%d)\n", okm, okl);

    do { r->cu = rnd(n); r->cv = rnd(n); } while (r->cu == r->cv);
    int cm = mg_has_edge(mg, r->cu, r->cv, &r->conn_m);
    int cl = lg_has_edge(lg, r->cu, r->cv, &r->conn_l);
    if (cm != cl) fprintf(stderr, "[경고] 연결 여부 결과 불일치 (matrix=%d list=%d)\n", cm, cl);
    r->connected = cm;

    r->au = rnd(n);
    r->nb_cnt_m = mg_neighbors(mg, r->au, r->nb_m, &r->adj_m);
    r->nb_cnt_l = lg_neighbors(lg, r->au, r->nb_l, &r->adj_l);
}

static void print_pairs(const SingleRun* r)
{
    int tmp_m[N_VERTICES], tmp_l[N_VERTICES];
    memcpy(tmp_m, r->nb_m, sizeof(tmp_m));
    memcpy(tmp_l, r->nb_l, sizeof(tmp_l));
    qsort(tmp_m, (size_t)r->nb_cnt_m, sizeof(int), cmp_int);
    qsort(tmp_l, (size_t)r->nb_cnt_l, sizeof(int), cmp_int);
    int same = (r->nb_cnt_m == r->nb_cnt_l) &&
        memcmp(tmp_m, tmp_l, (size_t)r->nb_cnt_m * sizeof(int)) == 0;

    printf("  측정에 사용한 정점 쌍 (인접 행렬과 인접 리스트에 똑같이 적용)\n");
    printf("    간선 삽입   : (%d, %d)   <- 삽입 전에는 없던 간선\n", r->iu, r->iv);
    printf("    간선 삭제   : (%d, %d)   <- 삭제 전에는 있던 간선\n", r->du, r->dv);
    printf("    연결 확인   : (%d, %d)   -> %s\n", r->cu, r->cv,
        r->connected ? "연결됨" : "연결되지 않음");
    printf("    인접 정점   : 정점 %d 의 인접 정점\n", r->au);
    print_vertices("인접 행렬 결과", r->nb_m, r->nb_cnt_m);
    print_vertices("인접 리스트 결과", r->nb_l, r->nb_cnt_l);
    printf("    (검증) 두 결과의 정점 집합이 동일한가 : %s\n", same ? "예" : "아니오!!");
}

static void print_case(int no, const char* gname, const char* rep, size_t mem,
    long ins, long del, long conn, long adj)
{
    char b[32];
    printf("Case %d : %s - %s\n", no, gname, rep);
    printf("%-30s : %s Bytes\n", "Memory", comma(mem, b));
    printf("%-30s : %ld\n", "Edge insertion comparisons", ins);
    printf("%-30s : %ld\n", "Edge deletion comparisons", del);
    printf("%-30s : %ld\n", "Connectivity check comparisons", conn);
    printf("%-30s : %ld\n\n", "Adjacent vertices comparisons", adj);
}

typedef struct {
    double ins_m, ins_l, del_m, del_l;
    double conn_m, conn_l;
    double conn_yes_m, conn_yes_l;
    double conn_no_m, conn_no_l;
    int    n_yes, n_no;
    double adj_m, adj_l;
} TrialStats;

static void run_trials(GraphSet* gs, TrialStats* s)
{
    MatrixGraph* mg = gs->mg;
    ListGraph* lg = gs->lg;
    int n = N_VERTICES;
    long dummy = 0;
    memset(s, 0, sizeof(*s));
    double sum_cy_m = 0, sum_cy_l = 0, sum_cn_m = 0, sum_cn_l = 0;

    for (int t = 0; t < TRIALS; t++) {
        int u, v;
        long cm, cl;

        do { u = rnd(n); v = rnd(n); } while (u == v || mg_peek(mg, u, v));
        cm = cl = 0;
        mg_insert_edge(mg, u, v, &cm);
        lg_insert_edge(lg, u, v, &cl);
        s->ins_m += (double)cm;  s->ins_l += (double)cl;
        mg_delete_edge(mg, u, v, &dummy);
        lg_delete_edge(lg, u, v, &dummy);

        do { u = rnd(n); v = rnd(n); } while (u == v || !mg_peek(mg, u, v));
        cm = cl = 0;
        mg_delete_edge(mg, u, v, &cm);
        lg_delete_edge(lg, u, v, &cl);
        s->del_m += (double)cm;  s->del_l += (double)cl;
        mg_insert_edge(mg, u, v, &dummy);
        lg_insert_edge(lg, u, v, &dummy);


        do { u = rnd(n); v = rnd(n); } while (u == v);
        cm = cl = 0;
        int ym = mg_has_edge(mg, u, v, &cm);
        lg_has_edge(lg, u, v, &cl);
        s->conn_m += (double)cm;  s->conn_l += (double)cl;
        if (ym) { sum_cy_m += (double)cm; sum_cy_l += (double)cl; s->n_yes++; }
        else { sum_cn_m += (double)cm; sum_cn_l += (double)cl; s->n_no++; }


        int buf[N_VERTICES];
        u = rnd(n);
        cm = cl = 0;
        mg_neighbors(mg, u, buf, &cm);
        lg_neighbors(lg, u, buf, &cl);
        s->adj_m += (double)cm;  s->adj_l += (double)cl;
    }

    s->ins_m /= TRIALS;  s->ins_l /= TRIALS;
    s->del_m /= TRIALS;  s->del_l /= TRIALS;
    s->conn_m /= TRIALS; s->conn_l /= TRIALS;
    s->adj_m /= TRIALS;  s->adj_l /= TRIALS;
    if (s->n_yes) { s->conn_yes_m = sum_cy_m / s->n_yes; s->conn_yes_l = sum_cy_l / s->n_yes; }
    if (s->n_no) { s->conn_no_m = sum_cn_m / s->n_no;  s->conn_no_l = sum_cn_l / s->n_no; }
}

static void print_trials(const GraphSet* gs, const TrialStats* s)
{
    printf("[%s]  (무작위 %d회 평균, 인접 행렬 vs 인접 리스트)\n", gs->name, TRIALS);
    printf("  %-34s %12s %12s\n", "", "Matrix", "List");
    printf("  %-34s %12.2f %12.2f\n", "Edge insertion", s->ins_m, s->ins_l);
    printf("  %-34s %12.2f %12.2f\n", "Edge deletion", s->del_m, s->del_l);
    printf("  %-34s %12.2f %12.2f\n", "Connectivity check (전체 평균)", s->conn_m, s->conn_l);
    printf("  %-34s %12.2f %12.2f   (%d쌍)\n", "  - 실제로 연결된 쌍",
        s->conn_yes_m, s->conn_yes_l, s->n_yes);
    printf("  %-34s %12.2f %12.2f   (%d쌍)\n", "  - 연결되지 않은 쌍",
        s->conn_no_m, s->conn_no_l, s->n_no);
    printf("  %-34s %12.2f %12.2f\n\n", "Adjacent vertices", s->adj_m, s->adj_l);
}

static void print_memory_table(const GraphSet* sp, const GraphSet* de)
{
    char b1[32], b2[32], b3[32], b4[32];
    printf("%-24s %14s %14s %14s %14s\n", "", "Sparse/Matrix", "Sparse/List",
        "Dense/Matrix", "Dense/List");
    printf("%-24s %14s %14s %14s %14s\n", "기본 부가 공간 (구조체)",
        comma(sp->mm.base, b1), comma(sp->ml.base, b2),
        comma(de->mm.base, b3), comma(de->ml.base, b4));
    printf("%-24s %14s %14s %14s %14s\n", "행렬 / 정점별 헤드 배열",
        comma(sp->mm.body, b1), comma(sp->ml.body, b2),
        comma(de->mm.body, b3), comma(de->ml.body, b4));
    printf("%-24s %14s %14s %14s %14s\n", "간선 노드",
        comma(sp->mm.nodes, b1), comma(sp->ml.nodes, b2),
        comma(de->mm.nodes, b3), comma(de->ml.nodes, b4));
    printf("%-24s %14s %14s %14s %14s\n", "합계 (Bytes)",
        comma(sp->mm.total, b1), comma(sp->ml.total, b2),
        comma(de->mm.total, b3), comma(de->ml.total, b4));
    if (HAVE_MALLINFO2) {
        printf("%-24s %14s %14s %14s %14s\n", "(참고) malloc 점유 포함",
            comma(sp->real_m, b1), comma(sp->real_l, b2),
            comma(de->real_m, b3), comma(de->real_l, b4));
    }
    printf("  간선 노드 1개 = %zu Bytes (정점 번호 + next 포인터),  행렬 칸 1개 = %zu Bytes\n",
        sizeof(AdjNode), sizeof(MatrixCell));
    printf("  (검증) 행렬의 1인 칸 수 = 2 x 간선 수, 리스트 노드 수 = 2 x 간선 수 : %s\n",
        (memory_consistent(sp) && memory_consistent(de)) ? "예" : "아니오!!");
}

int main(int argc, char* argv[])
{
    unsigned seed = (argc >= 2) ? (unsigned)atoi(argv[1]) : (unsigned)time(NULL);
    srand(seed);
    printf("난수 시드 : %u  (재현하려면 './graph_compare %u' 로 실행)\n\n", seed, seed);

    GraphSet sparse, dense;
    build_graph_set(&sparse, "Sparse Graph", SPARSE_EDGES, N_VERTICES);
    build_graph_set(&dense, "Dense Graph", DENSE_EDGES, N_VERTICES);

    printf("===== 1. 생성된 그래프 =====\n");
    print_graph_info(&sparse);
    printf("\n");
    print_graph_info(&dense);

    SingleRun rs, rd;
    run_single(&sparse, &rs);
    run_single(&dense, &rd);

    printf("\n===== 2. 측정 결과 (과제 출력 형식) =====\n\n");
    printf("----- Sparse Graph -----\n");
    print_pairs(&rs);
    printf("\n");
    print_case(1, "Sparse Graph", "Adjacency Matrix", sparse.mm.total,
        rs.ins_m, rs.del_m, rs.conn_m, rs.adj_m);
    print_case(2, "Sparse Graph", "Adjacency List", sparse.ml.total,
        rs.ins_l, rs.del_l, rs.conn_l, rs.adj_l);

    printf("----- Dense Graph -----\n");
    print_pairs(&rd);
    printf("\n");
    print_case(3, "Dense Graph", "Adjacency Matrix", dense.mm.total,
        rd.ins_m, rd.del_m, rd.conn_m, rd.adj_m);
    print_case(4, "Dense Graph", "Adjacency List", dense.ml.total,
        rd.ins_l, rd.del_l, rd.conn_l, rd.adj_l);

    printf("===== 3. 메모리 사용량 상세 =====\n");
    print_memory_table(&sparse, &dense);

    TrialStats ts, td;
    run_trials(&sparse, &ts);
    run_trials(&dense, &td);

    printf("\n===== 4. 추가 통계 : 위 단일 측정은 우연에 좌우되므로 %d회 평균도 함께 본다 =====\n", TRIALS);
    print_trials(&sparse, &ts);
    print_trials(&dense, &td);

    printf("(검증) 반복 측정 후에도 두 표현이 같은 그래프인가 : Sparse %s / Dense %s\n",
        graphs_equal(&sparse) ? "일치" : "불일치!!",
        graphs_equal(&dense) ? "일치" : "불일치!!");

    free_graph_set(&sparse);
    free_graph_set(&dense);
    return 0;
}
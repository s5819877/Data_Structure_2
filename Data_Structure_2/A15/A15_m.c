#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "A15_Util.h"
#include "A15_Counters.h"
#include "A15_QuickArray.h"
#include "A15_AVL.h"

#define NSIZES 6
#define MAXN   20000
#define MAXOPS (MAXN / 5)
#define MAXCAP (MAXN + MAXOPS)
static const int SIZES[NSIZES] = { 1000, 2000, 4000, 8000, 16000, 20000 };

typedef struct { int is_insert; int value; } Op;

static int bad_sorted = 0, bad_same = 0, bad_avl = 0, bad_diff = 0, bad_repeat = 0, runs = 0;

static void gen_ops(Rng* r, Op* ops, int nops, int range)
{
    for (int i = 0; i < nops; i++) {
        ops[i].is_insert = (rng_next(r) % 2u) == 0;
        ops[i].value = rng_range(r, 0, range);
    }
}
static void ops_rng(Rng* r, uint64_t seed, int n) { rng_seed(r, seed ^ 0x0A5A5A5A5ULL ^ ((uint64_t)n << 20)); }

typedef struct {
    Counters c;                 
    long long cmp1, mov1;       
    int ins, del_hit, del_skip; 
    int final_count, peak_len;  
    double t_total, t_phase1;   
} Result;

static void tally(Result* r, const Op* o, int removed)
{
    if (o->is_insert) r->ins++;
    else if (removed) r->del_hit++;
    else r->del_skip++;
}

static void run_quick(const int* data, int n, const Op* ops, int nops, int* w, Result* r)
{
    int len = n;
    memcpy(w, data, sizeof(int) * (size_t)n);
    memset(r, 0, sizeof * r);
    ctr_init(&r->c);
    r->peak_len = n;
    double t0 = now_ms();
    quick_sort(w, n, &r->c);
    double t1 = now_ms();
    r->cmp1 = r->c.cmp; r->mov1 = r->c.mov;
    for (int i = 0; i < nops; i++) {
        int removed = 0;
        if (ops[i].is_insert) { arr_insert(w, &len, ops[i].value, &r->c); if (len > r->peak_len) r->peak_len = len; }
        else removed = arr_delete(w, &len, ops[i].value, &r->c);
        tally(r, &ops[i], removed);
    }
    double t2 = now_ms();
    r->t_phase1 = t1 - t0; r->t_total = t2 - t0;
    r->final_count = len;
}

static void run_avl(const int* data, int n, const Op* ops, int nops, Result* r, AvlNode** keep)
{
    int len = n;
    AvlNode* root = NULL;
    memset(r, 0, sizeof * r);
    ctr_init(&r->c);
    r->peak_len = n;
    double t0 = now_ms();
    for (int i = 0; i < n; i++) root = avl_insert(root, data[i], &r->c);
    double t1 = now_ms();
    r->cmp1 = r->c.cmp; r->mov1 = r->c.mov;
    for (int i = 0; i < nops; i++) {
        int removed = 0;
        if (ops[i].is_insert) { root = avl_insert(root, ops[i].value, &r->c); len++; if (len > r->peak_len) r->peak_len = len; }
        else { root = avl_delete(root, ops[i].value, &removed, &r->c); if (removed) len--; }
        tally(r, &ops[i], removed);
    }
    double t2 = now_ms();
    r->t_phase1 = t1 - t0; r->t_total = t2 - t0;
    r->final_count = len;
    if (keep) *keep = root; else avl_free(root);
}

static unsigned long long mem_quick(const Result* r) { return (unsigned long long)r->peak_len * sizeof(int) + stack_model_bytes(&r->c); }
static unsigned long long mem_avl(const Result* r) { return (unsigned long long)r->c.bytes_peak + stack_model_bytes(&r->c); }

static void verify_run(const int* data, int n, const Op* ops, int nops, const Result* rq, const Result* ra)
{
    static int a[MAXCAP], tmp[MAXCAP];
    Counters cq, ca;
    ctr_init(&cq); ctr_init(&ca);
    AvlNode* root = NULL;
    int len = n;
    memcpy(a, data, sizeof(int) * (size_t)n);
    quick_sort(a, n, &cq);
    for (int i = 0; i < n; i++) root = avl_insert(root, data[i], &ca);
    runs += 2;
    if (!is_sorted(a, n)) bad_sorted++;
    if (!same_multiset(a, data, n)) bad_same++;
    if (!avl_valid(root)) bad_avl++;
    int k = avl_inorder(root, tmp);                                   
    if (k != n || memcmp(tmp, a, sizeof(int) * (size_t)n) != 0) bad_diff++;
    for (int i = 0; i < nops; i++) {
        int rem_a, rem_t;
        if (ops[i].is_insert) { arr_insert(a, &len, ops[i].value, &cq); root = avl_insert(root, ops[i].value, &ca); rem_a = rem_t = 1; }
        else { rem_a = arr_delete(a, &len, ops[i].value, &cq); root = avl_delete(root, ops[i].value, &rem_t, &ca); }
        if (rem_a != rem_t) bad_diff++;
        k = avl_inorder(root, tmp);                                   
        if (k != len || memcmp(tmp, a, sizeof(int) * (size_t)len) != 0) bad_diff++;
        if (!avl_valid(root)) bad_avl++;
    }
    if (!is_sorted(a, len)) bad_sorted++;
    if (cq.cmp != rq->c.cmp || cq.mov != rq->c.mov || ca.cmp != ra->c.cmp || ca.mov != ra->c.mov) bad_repeat++;   /* 같은 입력 = 같은 횟수 */
    if (len != rq->final_count || len != ra->final_count || avl_node_count(root) > len) bad_diff++;
    avl_free(root);
}

typedef struct { double mean, median; } TimeRes;
static int cmp_double(const void* a, const void* b) { double x = *(const double*)a, y = *(const double*)b; return (x > y) - (x < y); }
static void summarize(double* t, int rounds, TimeRes* res)
{
    double sum = 0;
    for (int i = 0; i < rounds; i++) sum += t[i];
    res->mean = sum / rounds;
    qsort(t, (size_t)rounds, sizeof(double), cmp_double);
    res->median = t[rounds / 2];
}

static void time_both(const int* data, int n, const Op* ops, int nops, int rounds, TimeRes tot[2], TimeRes ph1[2])
{
    static int w[MAXCAP];
    double* tt[2], * tp[2];
    for (int m = 0; m < 2; m++) { tt[m] = malloc(sizeof(double) * (size_t)rounds); tp[m] = malloc(sizeof(double) * (size_t)rounds); }
    Result r;
    for (int k = -3; k < rounds; k++) {
        for (int step = 0; step < 2; step++) {
            int m = (step + (k + 3)) % 2;                      /* 0 = 퀵 정렬, 1 = AVL */
            if (m == 0) run_quick(data, n, ops, nops, w, &r); else run_avl(data, n, ops, nops, &r, NULL);
            if (k >= 0) { tt[m][k] = r.t_total; tp[m][k] = r.t_phase1; }
        }
    }
    for (int m = 0; m < 2; m++) { summarize(tt[m], rounds, &tot[m]); summarize(tp[m], rounds, &ph1[m]); free(tt[m]); free(tp[m]); }
}

int main(int argc, char** argv)
{
    uint64_t seed = argc > 1 ? strtoull(argv[1], NULL, 10) : 20251022ULL;
    int trials = argc > 2 ? atoi(argv[2]) : 20;
    int time_rounds = argc > 3 ? atoi(argv[3]) : 30;
    if (trials < 1) trials = 1;

    static int data[MAXN], w[MAXCAP];
    static Op ops[MAXOPS];
    Rng dr, orng;
    rng_seed(&dr, seed);                                       
    Result rq[NSIZES], ra[NSIZES];
    TimeRes ttot[NSIZES][2], tph1[NSIZES][2];

    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s], nops = n / 5;
        gen_random(&dr, data, n);
        ops_rng(&orng, seed, n);
        gen_ops(&orng, ops, nops, VALUE_MAX);
        run_quick(data, n, ops, nops, w, &rq[s]);
        run_avl(data, n, ops, nops, &ra[s], NULL);
        verify_run(data, n, ops, nops, &rq[s], &ra[s]);
        if (time_rounds > 0) time_both(data, n, ops, nops, time_rounds, ttot[s], tph1[s]);
        else memset(ttot[s], 0, sizeof ttot[s]), memset(tph1[s], 0, sizeof tph1[s]);

        char b1[40], b2[40];
        printf("Data Size : %d\nOperations: %d\n\n", n, nops);
        printf("                  Quick Sort      AVL Tree\n");
        printf("------------------------------------------------\n");
        snprintf(b1, sizeof b1, "%.4f ms", ttot[s][0].mean); snprintf(b2, sizeof b2, "%.4f ms", ttot[s][1].mean);
        printf("%-18s%-16s%s\n", "Total Time", b1, b2);
        printf("%-18s%-16lld%lld\n", "Comparisons", rq[s].c.cmp, ra[s].c.cmp);
        printf("%-18s%-16lld%lld\n", "Data Movements", rq[s].c.mov, ra[s].c.mov);
        snprintf(b1, sizeof b1, "%llu Bytes", mem_quick(&rq[s])); snprintf(b2, sizeof b2, "%llu Bytes", mem_avl(&ra[s]));
        printf("%-18s%-16s%s\n", "Memory Usage", b1, b2);
        printf("\n%-18s%-16d%d\n\n", "Final Data Count", rq[s].final_count, ra[s].final_count);
    }

    printf("------------------------------------\n\n[추가 분석 1] 단계별 비교/이동 횟수 (위 1회 실험과 같은 데이터)\n");
    printf("정렬 = 퀵 정렬 / AVL 트리 구축.  연산 = 삽입·삭제 전체.  삽입, 삭제 성공, 삭제 건너뜀(값이 없음)\n");
    printf("%8s %6s %6s %6s | %10s %10s %10s %10s | %10s %10s %10s %10s\n", "n", "삽입", "삭제", "건너뜀", "퀵정렬비교", "퀵연산비교",
        "퀵정렬이동", "퀵연산이동", "AVL구축비교", "AVL연산비교", "AVL구축이동", "AVL연산이동");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d %6d %6d %6d | %10lld %10lld %10lld %10lld | %10lld %10lld %10lld %10lld\n", SIZES[s], rq[s].ins, rq[s].del_hit, rq[s].del_skip,
            rq[s].cmp1, rq[s].c.cmp - rq[s].cmp1, rq[s].mov1, rq[s].c.mov - rq[s].mov1,
            ra[s].cmp1, ra[s].c.cmp - ra[s].cmp1, ra[s].mov1, ra[s].c.mov - ra[s].mov1);
    printf("\n연산 1회당 평균 (연산 단계만)\n");
    printf("%8s | %12s %12s | %12s %12s\n", "n", "배열 비교", "배열 이동", "AVL 비교", "AVL 이동");
    for (int s = 0; s < NSIZES; s++) {
        double k = SIZES[s] / 5;
        printf("%8d | %12.1f %12.1f | %12.2f %12.3f\n", SIZES[s], (rq[s].c.cmp - rq[s].cmp1) / k, (rq[s].c.mov - rq[s].mov1) / k,
            (ra[s].c.cmp - ra[s].cmp1) / k, (ra[s].c.mov - ra[s].mov1) / k);
    }

    printf("\n------------------------------------\n\n[추가 분석 2] %d회 반복 평균 (매 회 새 데이터, 새 연산 순서)\n", trials);
    printf("%8s | %12s %12s %7s | %12s %12s %7s\n", "n", "퀵 비교", "AVL 비교", "퀵/AVL", "퀵 이동", "AVL 이동", "퀵/AVL");
    {
        Rng tr;
        rng_seed(&tr, seed + 1);
        for (int s = 0; s < NSIZES; s++) {
            int n = SIZES[s], nops = n / 5;
            double cq = 0, ca = 0, mq = 0, ma = 0;
            for (int t = 0; t < trials; t++) {
                Result a, b;
                gen_random(&tr, data, n);
                gen_ops(&tr, ops, nops, VALUE_MAX);
                run_quick(data, n, ops, nops, w, &a);
                run_avl(data, n, ops, nops, &b, NULL);
                if (!is_sorted(w, a.final_count)) bad_sorted++;
                runs += 2;
                cq += (double)a.c.cmp; ca += (double)b.c.cmp; mq += (double)a.c.mov; ma += (double)b.c.mov;
            }
            printf("%8d | %12.1f %12.1f %7.2f | %12.1f %12.1f %7.2f\n", n, cq / trials, ca / trials, cq / ca, mq / trials, ma / trials, mq / ma);
        }
    }

    if (time_rounds > 0) {
        printf("\n------------------------------------\n\n[추가 분석 3] 실행시간 (ms), 같은 입력으로 %d회 반복 (순서를 번갈아 실행)\n", time_rounds);
        printf("총 = 정렬(구축) + 삽입·삭제.  평균은 이따금 끼는 느린 측정값에 흔들리므로 중앙값도 함께 표시\n");
        printf("%8s | %9s %9s %9s %9s | %9s %9s %9s %9s\n", "n", "퀵 평균", "AVL 평균", "퀵 중앙", "AVL 중앙", "퀵 정렬", "AVL 구축", "퀵 연산", "AVL 연산");
        for (int s = 0; s < NSIZES; s++)
            printf("%8d | %9.4f %9.4f %9.4f %9.4f | %9.4f %9.4f %9.4f %9.4f\n", SIZES[s], ttot[s][0].mean, ttot[s][1].mean, ttot[s][0].median,
                ttot[s][1].median, tph1[s][0].median, tph1[s][1].median, ttot[s][0].median - tph1[s][0].median,
                ttot[s][1].median - tph1[s][1].median);
        printf("(오른쪽 네 열은 중앙값. 연산 = 총 - 정렬(구축))\n");
        printf("\n중앙값 기준 AVL 시간 / 퀵 시간 (1보다 작으면 AVL 이 더 빠름)\n");
        printf("%8s %10s %10s %10s\n", "n", "총", "정렬/구축", "연산");
        for (int s = 0; s < NSIZES; s++) {
            double qo = ttot[s][0].median - tph1[s][0].median, ao = ttot[s][1].median - tph1[s][1].median;
            printf("%8d %10.3f %10.3f %10.3f\n", SIZES[s], ttot[s][1].median / ttot[s][0].median, tph1[s][1].median / tph1[s][0].median, ao / qo);
        }
    }

    printf("\n------------------------------------\n\n[추가 분석 4] 메모리 구성 (Byte)\n");
    printf("배열 = 원소 최대 개수 x %d + 퀵 정렬 스택(계산).  AVL = 노드 최대 개수 x %d + 재귀 스택(계산)\n", (int)sizeof(int), (int)sizeof(AvlNode));
    printf("스택(계산) = (최대 재귀 깊이 + 1) x %d\n", FRAME_BYTES_MODEL);
    printf("%8s | %8s %9s %6s %9s %9s | %8s %9s %6s %9s %9s | %8s\n", "n", "최대원소", "배열", "깊이", "스택", "합계", "최대노드", "노드", "깊이", "스택", "합계", "AVL/배열");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d | %8d %9llu %6d %9llu %9llu | %8zu %9zu %6d %9llu %9llu | %8.2f\n", SIZES[s], rq[s].peak_len,
            (unsigned long long)rq[s].peak_len * sizeof(int), rq[s].c.max_depth, stack_model_bytes(&rq[s].c), mem_quick(&rq[s]),
            ra[s].c.bytes_peak / sizeof(AvlNode), ra[s].c.bytes_peak, ra[s].c.max_depth, stack_model_bytes(&ra[s].c), mem_avl(&ra[s]),
            (double)mem_avl(&ra[s]) / (double)mem_quick(&rq[s]));

    printf("\n------------------------------------\n\n[추가 분석 5] 값 범위를 0..2n 으로 줄여 삭제가 실제로 일어나고 중복이 생기는 경우\n");
    printf("%8s %6s %6s %6s %7s %7s | %10s %10s | %10s %10s | %7s %5s\n", "n", "삽입", "삭제", "건너뜀", "최종수", "노드수", "퀵 비교",
        "퀵 이동", "AVL 비교", "AVL 이동", "회전", "높이");
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s], nops = n / 5, range = 2 * n;
        Rng r5;
        Result a, b;
        AvlNode* root;
        rng_seed(&r5, (seed ^ 0x2D2DULL) + (uint64_t)n);
        for (int i = 0; i < n; i++) data[i] = rng_range(&r5, 0, range);
        ops_rng(&orng, seed, n);
        gen_ops(&orng, ops, nops, range);
        run_quick(data, n, ops, nops, w, &a);
        run_avl(data, n, ops, nops, &b, &root);
        verify_run(data, n, ops, nops, &a, &b);
        printf("%8d %6d %6d %6d %7d %7lld | %10lld %10lld | %10lld %10lld | %7lld %5d\n", n, a.ins, a.del_hit, a.del_skip, a.final_count,
            avl_node_count(root), a.c.cmp, a.c.mov, b.c.cmp, b.c.mov, b.c.rot, avl_height(root));
        avl_free(root);
    }

    printf("\n[자체 검증] 실행 %d회: 정렬 위반 %d, 데이터 유실 %d, AVL 규칙 위반 %d, 두 구조의 구성 불일치 %d, 반복 실행 횟수 불일치 %d\n",
        runs, bad_sorted, bad_same, bad_avl, bad_diff, bad_repeat);
    printf("[시드] %llu  (연산 난수는 시드 ^ 0x0A5A5A5A5 ^ (n << 20) 로 별도 생성)\n", (unsigned long long)seed);
    return (bad_sorted || bad_same || bad_avl || bad_diff || bad_repeat) ? 1 : 0;
}
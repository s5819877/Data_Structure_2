#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "A14_Sorts.h"
#include "A14_Util.h"

#define NSIZES 6
#define MAXN   20000
static const int SIZES[NSIZES] = { 1000, 2000, 4000, 8000, 16000, 20000 };
enum { ALG_HEAP, ALG_MERGE, ALG_QF, ALG_QR, ALG_QM, NALG };
static const char* LABEL[NALG] = { "Heap Sort", "Merge Sort", "Quick (First)", "Quick (Random)", "Quick (Median3)" };

static int bad_sorted = 0, bad_multiset = 0, bad_bounds = 0, bad_heap = 0, runs = 0;
static uint64_t pivot_seed_base;

static long long merge_best(int n) { return n < 2 ? 0 : merge_best(n / 2) + merge_best(n - n / 2) + n / 2; }
static long long merge_worst(int n) { return n < 2 ? 0 : merge_worst(n / 2) + merge_worst(n - n / 2) + (n - 1); }

static int ilog2(int x) { int r = 0; while (x > 1) { x >>= 1; r++; } return r; }
static long long heap_worst(int n)
{
    if (n < 2) return 0;
    long long w = 0;
    int h = ilog2(n);
    for (int i = (n - 2) / 2; i >= 0; i--) w += 2LL * (h - ilog2(i + 1));   
    for (int m = n - 1; m >= 1; m--)        w += 2LL * ilog2(m);            
    return w;
}

static void run_alg(int alg, int* a, int n, Rng* pr, SortStats* st)
{
    if (alg == ALG_HEAP)       heap_sort(a, n, st);
    else if (alg == ALG_MERGE) merge_sort(a, n, st);
    else                       quick_sort(a, n, (PivotMethod)(alg - ALG_QF), pr, st);
}

static void measure_once(int alg, const int* src, int n, int probe_stack, SortStats* st)
{
    static int w[MAXN];
    Rng pr;
    memcpy(w, src, sizeof(int) * n);
    rng_seed(&pr, pivot_seed_base);
    stats_init(st, probe_stack);
    run_alg(alg, w, n, &pr, st);
    runs++;
    if (!is_sorted(w, n))          bad_sorted++;
    if (!same_multiset(w, src, n)) bad_multiset++;
    if (alg == ALG_MERGE) {
        if (st->cmp < merge_best(n) || st->cmp > merge_worst(n)) bad_bounds++;
        if (st->heap_bytes != (n >= 2 ? sizeof(int) * (size_t)n : 0)) bad_heap++;     
    }
    else if (st->heap_bytes != 0) bad_heap++;                                       
    if (alg == ALG_HEAP && (st->cmp > heap_worst(n) || st->cmp_build > st->cmp)) bad_bounds++;
}

typedef struct { double mean, median; } TimeRes;
static int cmp_double(const void* a, const void* b)
{
    double x = *(const double*)a, y = *(const double*)b;
    return (x > y) - (x < y);
}
static void summarize(double* t, int rounds, TimeRes* res)
{
    double sum = 0;
    for (int i = 0; i < rounds; i++) sum += t[i];
    res->mean = sum / rounds;
    qsort(t, (size_t)rounds, sizeof(double), cmp_double);
    res->median = t[rounds / 2];
}

static TimeRes time_same(int alg, const int* src, int n, int rounds)
{
    static int w[MAXN];
    TimeRes res;
    double* t = (double*)malloc(sizeof(double) * rounds);
    Rng pr;
    SortStats st;
    for (int r = -3; r < rounds; r++) {                 
        memcpy(w, src, sizeof(int) * n);
        rng_seed(&pr, pivot_seed_base);
        stats_init(&st, 0);
        double t0 = now_ms();
        run_alg(alg, w, n, &pr, &st);
        double t1 = now_ms();
        if (!is_sorted(w, n)) bad_sorted++;
        runs++;
        if (r >= 0) t[r] = t1 - t0;
    }
    summarize(t, rounds, &res);
    free(t);
    return res;
}

static void time_fresh(int n, int rounds, uint64_t seed, TimeRes res[NALG])
{
    static int data[MAXN], w[MAXN];
    double* t[NALG];
    Rng dr, pr;
    SortStats st;
    for (int a = 0; a < NALG; a++) t[a] = (double*)malloc(sizeof(double) * rounds);
    rng_seed(&dr, seed);
    for (int r = -3; r < rounds; r++) {
        gen_random(&dr, data, n);
        for (int step = 0; step < NALG; step++) {
            int alg = (step + (r + 3)) % NALG;
            memcpy(w, data, sizeof(int) * n);
            rng_seed(&pr, pivot_seed_base);
            stats_init(&st, 0);
            double t0 = now_ms();
            run_alg(alg, w, n, &pr, &st);
            double t1 = now_ms();
            if (!is_sorted(w, n)) bad_sorted++;
            runs++;
            if (r >= 0) t[alg][r] = t1 - t0;
        }
    }
    for (int a = 0; a < NALG; a++) { summarize(t[a], rounds, &res[a]); free(t[a]); }
}

int main(int argc, char** argv)
{
    uint64_t seed = argc > 1 ? strtoull(argv[1], NULL, 10) : 20251022ULL;
    int trials = argc > 2 ? atoi(argv[2]) : 20;
    int time_rounds = argc > 3 ? atoi(argv[3]) : 100;
    if (trials < 1) trials = 1;
    pivot_seed_base = seed ^ 0x5DEECE66DULL;            

    static int data[MAXN];
    Rng dr;
    rng_seed(&dr, seed);

    SortStats res[NSIZES][NALG], probe[NSIZES][NALG];

    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        gen_random(&dr, data, n);
        for (int a = 0; a < NALG; a++) {
            measure_once(a, data, n, 0, &res[s][a]);          
            measure_once(a, data, n, 1, &probe[s][a]);        
            if (probe[s][a].cmp != res[s][a].cmp) bad_bounds++;   
        }
        printf("Data Size : %d\n\n", n);
        printf("Algorithm       Comparisons    Time(ms)    Memory(Bytes)\n");
        printf("--------------------------------------------------------\n");
        for (int a = 0; a < NALG; a++) {
            double ms = 0;
            if (time_rounds > 0) ms = time_same(a, data, n, time_rounds).mean;
            printf("%-16s%-15lld%-12.4f%llu\n", LABEL[a], res[s][a].cmp, ms, memory_model_bytes(&res[s][a]));
        }
        printf("\n");
    }

    double avg_cmp[NSIZES][NALG];
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        double c[NALG] = { 0 };
        for (int t = 0; t < trials; t++) {
            gen_random(&dr, data, n);
            for (int a = 0; a < NALG; a++) { SortStats st; measure_once(a, data, n, 0, &st); c[a] += (double)st.cmp; }
        }
        for (int a = 0; a < NALG; a++) avg_cmp[s][a] = c[a] / trials;
    }
    printf("------------------------------------\n\n[추가 분석 1] %d회 반복 평균 비교 횟수 (매 회 새 데이터)\n", trials);
    printf("%8s %10s %10s %10s %10s %10s | %6s %6s %6s %6s %6s\n", "n", "Heap", "Merge", "First", "Random", "Median3",
        "H/nlgn", "M/nlgn", "F/nlgn", "R/nlgn", "Md/nlgn");
    for (int s = 0; s < NSIZES; s++) {
        double nl = SIZES[s] * log2((double)SIZES[s]);
        printf("%8d", SIZES[s]);
        for (int a = 0; a < NALG; a++) printf(" %10.1f", avg_cmp[s][a]);
        printf(" |");
        for (int a = 0; a < NALG; a++) printf(" %6.3f", avg_cmp[s][a] / nl);
        printf("\n");
    }
    printf("\n병합 정렬 대비 비교 횟수 비 (1보다 크면 병합 정렬보다 비교를 더 많이 함)\n");
    printf("%8s %10s %10s %10s %10s    병합 정렬의 이론 범위 [최소, 최대]\n", "n", "Heap", "First", "Random", "Median3");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d %10.3f %10.3f %10.3f %10.3f    [%lld, %lld]\n", SIZES[s], avg_cmp[s][ALG_HEAP] / avg_cmp[s][ALG_MERGE],
            avg_cmp[s][ALG_QF] / avg_cmp[s][ALG_MERGE], avg_cmp[s][ALG_QR] / avg_cmp[s][ALG_MERGE],
            avg_cmp[s][ALG_QM] / avg_cmp[s][ALG_MERGE], merge_best(SIZES[s]), merge_worst(SIZES[s]));
    printf("\n비교 횟수가 n^p 에 비례한다고 볼 때의 지수 p  (인접한 두 크기 사이)\n");
    printf("%14s %9s %9s %9s %9s %9s\n", "구간", "Heap", "Merge", "First", "Random", "Median3");
    for (int s = 1; s < NSIZES; s++) {
        double dn = log((double)SIZES[s] / SIZES[s - 1]);
        printf("%6d->%-6d", SIZES[s - 1], SIZES[s]);
        for (int a = 0; a < NALG; a++) printf(" %9.3f", log(avg_cmp[s][a] / avg_cmp[s - 1][a]) / dn);
        printf("\n");
    }

    printf("\n힙 정렬 비교 횟수의 구성 (각 크기의 1회 실험 데이터)\n");
    printf("%8s %10s %10s %10s %8s %8s %12s\n", "n", "힙 구성", "정렬 단계", "합계", "구성/n", "정렬/nlgn", "이론 최대");
    for (int s = 0; s < NSIZES; s++) {
        double n = SIZES[s];
        long long b = res[s][ALG_HEAP].cmp_build, t = res[s][ALG_HEAP].cmp;
        printf("%8d %10lld %10lld %10lld %8.3f %8.3f %12lld\n", SIZES[s], b, t - b, t, b / n, (t - b) / (n * log2(n)), heap_worst(SIZES[s]));
    }

    if (time_rounds > 0) {
        TimeRes tr[NSIZES][NALG];
        for (int s = 0; s < NSIZES; s++) time_fresh(SIZES[s], time_rounds, seed + 1, tr[s]);
        printf("\n------------------------------------\n\n[추가 분석 2] 실행시간 (ms), 매 회 새 데이터로 %d회 측정\n", time_rounds);
        printf("평균은 이따금 끼는 느린 측정값(운영체제 잡음)에 흔들리므로 중앙값도 함께 표시\n\n[평균] (ms)\n");
        printf("%8s | %9s %9s %9s %9s %9s\n", "n", "Heap", "Merge", "First", "Random", "Median3");
        for (int s = 0; s < NSIZES; s++) {
            printf("%8d |", SIZES[s]);
            for (int a = 0; a < NALG; a++) printf(" %9.4f", tr[s][a].mean);
            printf("\n");
        }
        printf("\n[중앙값] (ms)\n");
        printf("%8s | %9s %9s %9s %9s %9s\n", "n", "Heap", "Merge", "First", "Random", "Median3");
        for (int s = 0; s < NSIZES; s++) {
            printf("%8d |", SIZES[s]);
            for (int a = 0; a < NALG; a++) printf(" %9.4f", tr[s][a].median);
            printf("\n");
        }
        printf("\n중앙값 기준 시간 비 (병합 정렬 = 1 일 때, 1보다 작으면 병합 정렬보다 빠름)\n");
        printf("%8s %10s %10s %10s %10s\n", "n", "Heap", "First", "Random", "Median3");
        for (int s = 0; s < NSIZES; s++)
            printf("%8d %10.3f %10.3f %10.3f %10.3f\n", SIZES[s], tr[s][ALG_HEAP].median / tr[s][ALG_MERGE].median,
                tr[s][ALG_QF].median / tr[s][ALG_MERGE].median, tr[s][ALG_QR].median / tr[s][ALG_MERGE].median,
                tr[s][ALG_QM].median / tr[s][ALG_MERGE].median);
        printf("\n중앙값 기준 힙 정렬 시간 / Median3 퀵 정렬 시간\n");
        for (int s = 0; s < NSIZES; s++) printf("%8d %10.3f\n", SIZES[s], tr[s][ALG_HEAP].median / tr[s][ALG_QM].median);
    }

    printf("\n------------------------------------\n\n[추가 분석 3] 메모리 구성 (각 크기의 1회 실험 데이터, Byte)\n");
    printf("추가 배열 = 정렬 중 malloc 으로 잡은 양을 직접 센 값\n");
    printf("스택(계산) = (최대 재귀 깊이 + 1) x %d   ← 표의 Memory 는 추가 배열 + 스택(계산)\n", FRAME_BYTES_MODEL);
    printf("스택(실측) = 이 빌드에서 실제로 스택이 얼마나 내려갔는지 잰 값 (컴파일러/최적화 옵션마다 다름)\n");
    printf("%8s %-16s %10s %6s %10s %10s %10s\n", "n", "알고리즘", "추가 배열", "깊이", "스택(계산)", "합계", "스택(실측)");
    for (int s = 0; s < NSIZES; s++) {
        for (int a = 0; a < NALG; a++)
            printf("%8d %-16s %10llu %6d %10llu %10llu %10llu\n", SIZES[s], LABEL[a], (unsigned long long)res[s][a].heap_bytes,
                res[s][a].max_depth, stack_model_bytes(&res[s][a]), memory_model_bytes(&res[s][a]),
                (unsigned long long)probe[s][a].stack_bytes);
        printf("%8s %-16s 병합 정렬 메모리 / 힙 정렬 메모리 = %.1f 배,  퀵(Median3) 메모리 / 힙 정렬 메모리 = %.2f 배\n", "", "",
            (double)memory_model_bytes(&res[s][ALG_MERGE]) / (double)memory_model_bytes(&res[s][ALG_HEAP]),
            (double)memory_model_bytes(&res[s][ALG_QM]) / (double)memory_model_bytes(&res[s][ALG_HEAP]));
    }

    printf("\n[자체 검증] 정렬 실행 %d회: 오름차순 위반 %d, 데이터 유실/변형 %d, 병합 비교 범위/측정 오류 %d, 추가 메모리 불일치 %d\n",
        runs, bad_sorted, bad_multiset, bad_bounds, bad_heap);
    printf("[시드] %llu  (피벗 난수는 시드 ^ 0x5DEECE66D 로 별도 생성)\n", (unsigned long long)seed);
    return (bad_sorted || bad_multiset || bad_bounds || bad_heap) ? 1 : 0;
}
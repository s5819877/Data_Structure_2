#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "A12_Sorts.h"
#include "A12_Util.h"
#ifdef _WIN32                      
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define NSIZES 6
#define MAXN   20000
static const int SIZES[NSIZES] = { 1000, 2000, 4000, 8000, 16000, 20000 };
static const char* LABEL[3] = { "Quick Sort (First)   ", "Quick Sort (Random)  ", "Quick Sort (Median-3)" };

static int bad_sorted = 0, bad_multiset = 0, bad_iter = 0, runs = 0;
static Range g_stack[MAXN + 2];
static uint64_t pivot_seed_base;

static QSStats run_pair(const int* src, int n, PivotMethod m)
{
    static int w1[MAXN], w2[MAXN];
    QSStats s1, s2;
    Rng pr;
    memcpy(w1, src, sizeof(int) * n);                 
    memcpy(w2, src, sizeof(int) * n);
    rng_seed(&pr, pivot_seed_base);
    quick_sort_recursive(w1, n, m, &pr, &s1);
    rng_seed(&pr, pivot_seed_base);
    quick_sort_iterative(w2, n, m, &pr, &s2, g_stack);
    runs += 2;
    if (!is_sorted(w1, n) || !is_sorted(w2, n))             bad_sorted++;
    if (!same_multiset(w1, src, n) || !same_multiset(w2, src, n)) bad_multiset++;
    if (s1.cmp != s2.cmp || memcmp(w1, w2, sizeof(int) * n) != 0 || s1.max_depth != s2.max_depth) {
        bad_iter++;
        fprintf(stderr, "[재귀/반복 불일치] n=%d %s %lld vs %lld\n", n, pivot_name(m), s1.cmp, s2.cmp);
    }
    return s1;
}

static QSStats run_iter_only(const int* src, int n, PivotMethod m)
{
    static int w[MAXN];
    QSStats st;
    Rng pr;
    memcpy(w, src, sizeof(int) * n);
    rng_seed(&pr, pivot_seed_base);
    quick_sort_iterative(w, n, m, &pr, &st, g_stack);
    runs++;
    if (!is_sorted(w, n))          bad_sorted++;
    if (!same_multiset(w, src, n)) bad_multiset++;
    return st;
}

#ifdef _WIN32
static double now_ms(void)
{
    static LARGE_INTEGER freq;
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)freq.QuadPart;
}
#else
static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}
#endif
static int cmp_double(const void* a, const void* b)
{
    double x = *(const double*)a, y = *(const double*)b;
    return (x > y) - (x < y);
}
typedef struct { double mean[2], median[2], min[2]; } TimeRes;    

static void time_pair(int n, PivotMethod m, int rounds, uint64_t seed, TimeRes* res)
{
    static int data[MAXN], w[MAXN];
    double* t[2] = { malloc(sizeof(double) * rounds), malloc(sizeof(double) * rounds) };
    Rng dr, pr;
    QSStats st[2];
    rng_seed(&dr, seed);
    for (int r = -5; r < rounds; r++) {               
        gen_random(&dr, data, n);
        for (int k = 0; k < 2; k++) {
            int which = (r % 2 == 0) ? k : 1 - k;     
            memcpy(w, data, sizeof(int) * n);
            rng_seed(&pr, pivot_seed_base);
            double t0 = now_ms();
            if (which == 0) quick_sort_recursive(w, n, m, &pr, &st[0]);
            else            quick_sort_iterative(w, n, m, &pr, &st[1], g_stack);
            double t1 = now_ms();
            if (!is_sorted(w, n)) bad_sorted++;
            runs++;
            if (r >= 0) t[which][r] = t1 - t0;
        }
        if (st[0].cmp != st[1].cmp) bad_iter++;
    }
    for (int k = 0; k < 2; k++) {
        double sum = 0, mn = t[k][0];
        for (int r = 0; r < rounds; r++) { sum += t[k][r]; if (t[k][r] < mn) mn = t[k][r]; }
        res->mean[k] = sum / rounds;
        res->min[k] = mn;
        qsort(t[k], (size_t)rounds, sizeof(double), cmp_double);
        res->median[k] = t[k][rounds / 2];
        free(t[k]);
    }
}

int main(int argc, char** argv)
{
    uint64_t seed = argc > 1 ? strtoull(argv[1], NULL, 10) : 20251022ULL;
    int trials = argc > 2 ? atoi(argv[2]) : 20;
    int time_rounds = argc > 3 ? atoi(argv[3]) : 300;
    if (trials < 1) trials = 1;
    pivot_seed_base = seed ^ 0x5DEECE66DULL;          

    static int data[MAXN], tmp[MAXN];
    Rng dr;
    rng_seed(&dr, seed);

    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        gen_random(&dr, data, n);
        printf("Data Size : %d\n\n", n);
        for (int m = 0; m < 3; m++)
            printf("%s : %lld comparisons\n", LABEL[m], run_pair(data, n, (PivotMethod)m).cmp);
        printf("\n");
    }

    double avg_cmp[NSIZES][3], avg_depth[NSIZES][3], avg_bal[NSIZES][3];
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        double c[3] = { 0 }, d[3] = { 0 }, b[3] = { 0 };
        for (int t = 0; t < trials; t++) {
            gen_random(&dr, data, n);
            for (int m = 0; m < 3; m++) {
                QSStats st = run_pair(data, n, (PivotMethod)m);
                c[m] += (double)st.cmp;
                d[m] += st.max_depth;
                b[m] += st.big_parts ? st.big_balance / st.big_parts : 0;
            }
        }
        for (int m = 0; m < 3; m++) { avg_cmp[s][m] = c[m] / trials; avg_depth[s][m] = d[m] / trials; avg_bal[s][m] = b[m] / trials; }
    }
    printf("------------------------------------\n\n[추가 분석 1] %d회 반복 평균 비교 횟수\n", trials);
    printf("%8s %14s %14s %14s | %9s %9s %9s\n", "n", "First", "Random", "Median-3", "F/n·lg n", "R/n·lg n", "M/n·lg n");
    for (int s = 0; s < NSIZES; s++) {
        double nl = SIZES[s] * log2((double)SIZES[s]);
        printf("%8d %14.1f %14.1f %14.1f | %9.3f %9.3f %9.3f\n", SIZES[s], avg_cmp[s][0], avg_cmp[s][1], avg_cmp[s][2],
            avg_cmp[s][0] / nl, avg_cmp[s][1] / nl, avg_cmp[s][2] / nl);
    }
    printf("\nMedian-3 가 First 보다 줄인 비율 / Random 이 First 보다 줄인 비율\n");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d   Median-3: %5.1f%%   Random: %5.1f%%\n", SIZES[s],
            100.0 * (1 - avg_cmp[s][2] / avg_cmp[s][0]), 100.0 * (1 - avg_cmp[s][1] / avg_cmp[s][0]));

    printf("\n비교 횟수가 n^p 에 비례한다고 볼 때의 지수 p  (인접한 두 크기 사이)\n");
    printf("%14s %10s %10s %10s\n", "구간", "First", "Random", "Median-3");
    for (int s = 1; s < NSIZES; s++) {
        double dn = log((double)SIZES[s] / SIZES[s - 1]);
        printf("%6d->%-6d", SIZES[s - 1], SIZES[s]);
        for (int m = 0; m < 3; m++) printf(" %10.3f", log(avg_cmp[s][m] / avg_cmp[s - 1][m]) / dn);
        printf("\n");
    }

    printf("\n[추가 분석 2] 분할 균형도와 최대 재귀 깊이 (%d회 평균)\n", trials);
    printf("균형도 = 크기 64 이상인 구간을 분할할 때 min(왼쪽, 오른쪽)/(구간 크기-1) 의 평균 (0.5 = 정확히 절반)\n");
    printf("%8s | %8s %8s %8s | %8s %8s %8s\n", "n", "F 균형", "R 균형", "M 균형", "F 깊이", "R 깊이", "M 깊이");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d | %8.3f %8.3f %8.3f | %8.1f %8.1f %8.1f\n", SIZES[s], avg_bal[s][0], avg_bal[s][1], avg_bal[s][2],
            avg_depth[s][0], avg_depth[s][1], avg_depth[s][2]);

    printf("\n[추가 분석 3] 이미 정렬된 입력 / 역순 입력 (과제 요구 외 참고 실험, 크기별 1회)\n");
    printf("비교 횟수\n%8s | %12s %10s %10s | %12s %10s %10s\n", "n", "정렬:First", "Random", "Median-3", "역순:First", "Random", "Median-3");
    long long cm[NSIZES][2][3]; int dp[NSIZES][2][3];
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        gen_random(&dr, data, n);
        for (int kind = 0; kind < 2; kind++) {
            if (kind == 0) gen_sorted(data, tmp, n); else gen_reverse(data, tmp, n);
            for (int m = 0; m < 3; m++) { QSStats st = run_iter_only(tmp, n, (PivotMethod)m); cm[s][kind][m] = st.cmp; dp[s][kind][m] = st.max_depth; }
        }
        printf("%8d | %12lld %10lld %10lld | %12lld %10lld %10lld\n", n, cm[s][0][0], cm[s][0][1], cm[s][0][2],
            cm[s][1][0], cm[s][1][1], cm[s][1][2]);
    }
    printf("최대 재귀 깊이\n%8s | %12s %10s %10s | %12s %10s %10s\n", "n", "정렬:First", "Random", "Median-3", "역순:First", "Random", "Median-3");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d | %12d %10d %10d | %12d %10d %10d\n", SIZES[s], dp[s][0][0], dp[s][0][1], dp[s][0][2],
            dp[s][1][0], dp[s][1][1], dp[s][1][2]);

    if (time_rounds > 0) {
        TimeRes tr;
        printf("\n------------------------------------\n\n[보너스] 재귀적 vs 반복적 퀵 정렬 (실행시간, %d회 측정 평균)\n\n", time_rounds);
        time_pair(10000, PIVOT_MEDIAN3, time_rounds, seed + 1, &tr);
        printf("Data Size : 10000\nPivot     : Median-of-Three\n\n");
        printf("Recursive Quick Sort : %.4f ms\n", tr.mean[0]);
        printf("Iterative Quick Sort : %.4f ms\n", tr.mean[1]);
        printf("\n(참고) 중앙값 / 최솟값 : 재귀 %.4f / %.4f ms,  반복 %.4f / %.4f ms\n",
            tr.median[0], tr.min[0], tr.median[1], tr.min[1]);

        printf("\n[보너스 추가] 피벗 방법과 크기별 실행시간 (ms)  -- 평균은 이따금 끼는 느린 측정값(운영체제 잡음)에 흔들리므로 중앙값도 함께 표시\n");
        printf("%-9s %7s | %9s %9s %8s | %9s %9s %8s\n", "피벗", "n", "평균 재귀", "평균 반복", "반복/재귀", "중앙 재귀", "중앙 반복", "반복/재귀");
        for (int m = 0; m < 3; m++) {
            static const int tn[3] = { 1000, 10000, 20000 };
            for (int k = 0; k < 3; k++) {
                time_pair(tn[k], (PivotMethod)m, time_rounds, seed + 1, &tr);
                printf("%-9s %7d | %9.4f %9.4f %8.3f | %9.4f %9.4f %8.3f\n", pivot_name((PivotMethod)m), tn[k],
                    tr.mean[0], tr.mean[1], tr.mean[1] / tr.mean[0],
                    tr.median[0], tr.median[1], tr.median[1] / tr.median[0]);
            }
        }
    }

    printf("\n[자체 검증] 정렬 실행 %d회: 오름차순 위반 %d, 데이터 유실/변형 %d, 재귀/반복 결과·횟수 불일치 %d\n",
        runs, bad_sorted, bad_multiset, bad_iter);
    printf("[시드] %llu  (피벗 난수는 시드 ^ 0x5DEECE66D 로 별도 생성)\n", (unsigned long long)seed);
    return (bad_sorted || bad_multiset || bad_iter) ? 1 : 0;
}
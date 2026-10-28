/* ==========================================================================
 *  main.c  —  과제-11 : 삽입 정렬과 쉘 정렬(기본 간격 / Knuth 간격)의 비교 횟수 비교
 *
 *  사용법 : ./shell_compare [seed] [trials]
 *           기본값  seed=20251022  trials=20
 *  앞부분은 과제 4장 형식(데이터 크기별 1회 실험)이고, 그 뒤에 추가 분석
 *  (간격 수열, 여러 번 반복한 평균, 크기에 따른 증가 지수)과 자체 검증 결과가 붙는다.
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "A11_Sorts.h"
#include "A11_Util.h"

#define NSIZES 6
#define MAXN   20000
static const int SIZES[NSIZES] = { 1000, 2000, 4000, 8000, 16000, 20000 };
enum { INS, SHO, SHK, NALG };
static const char* LABEL[NALG] = { "Insertion Sort       ", "Shell Sort (Original)", "Shell Sort (Knuth)   " };
typedef long long (*SortFn)(int*, int);
static const SortFn FN[NALG] = { insertion_sort, shell_sort_original, shell_sort_knuth };

static int bad_sorted = 0, bad_multiset = 0, bad_theory = 0, runs = 0;

/* 같은 원본 src 의 복사본에 alg 번 정렬을 수행하고 결과를 검증 */
static long long run(int alg, const int* src, int n)
{
    static int w[MAXN];
    memcpy(w, src, sizeof(int) * n);           /* 복사는 횟수에 미포함 */
    long long c = FN[alg](w, n);
    runs++;
    if (!is_sorted(w, n))          bad_sorted++;
    if (!same_multiset(w, src, n)) bad_multiset++;
    return c;
}

static void print_gaps(const char* title, int n)
{
    int g[MAX_GAPS], c;
    printf("%s (n = %d)\n", title, n);
    c = gaps_original(n, g);
    printf("  Original (%2d단계):", c);
    for (int i = 0; i < c; i++) printf(" %d", g[i]);
    c = gaps_knuth(n, g);
    printf("\n  Knuth    (%2d단계):", c);
    for (int i = 0; i < c; i++) printf(" %d", g[i]);
    printf("\n");
}

int main(int argc, char** argv)
{
    uint64_t seed = argc > 1 ? strtoull(argv[1], NULL, 10) : 20251022ULL;
    int trials = argc > 2 ? atoi(argv[2]) : 20;
    if (trials < 1) trials = 1;
    rng_seed(seed);

    static int data[MAXN];

    /* ---------------- 데이터 크기별 1회 실험 (과제 4장 형식) ---------------- */
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        gen_random(data, n);
        long long c[NALG];
        for (int a = 0; a < NALG; a++) c[a] = run(a, data, n);
        if (c[INS] != insertion_cmp_expected(data, n)) {       /* 정렬 코드와 무관한 직접 계산과 대조 */
            bad_theory++;
            fprintf(stderr, "[이론값 불일치] n=%d 삽입 정렬 측정 %lld\n", n, c[INS]);
        }
        printf("Data Size : %d\n\n", n);
        for (int a = 0; a < NALG; a++) printf("%s: %lld comparisons\n", LABEL[a], c[a]);
        printf("\n");
    }

    /* ---------------- 추가 분석 1 : 간격 수열 ---------------- */
    printf("------------------------------------\n\n[추가 분석 1] 간격 수열\n");
    print_gaps("", 1000);
    print_gaps("", 20000);

    /* ---------------- 추가 분석 2 : 여러 번 반복한 평균 ---------------- */
    fprintf(stderr, "반복 평균 실험: 크기 %d개 x %d회 (몇 초 걸립니다)\n", NSIZES, trials);
    double avg[NSIZES][NALG];
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        double sum[NALG] = { 0, 0, 0 };
        for (int t = 0; t < trials; t++) {
            gen_random(data, n);                         /* 매 회 새 데이터, 세 정렬은 같은 복사본 */
            for (int a = 0; a < NALG; a++) sum[a] += (double)run(a, data, n);
        }
        for (int a = 0; a < NALG; a++) avg[s][a] = sum[a] / trials;
    }
    printf("\n------------------------------------\n\n[추가 분석 2] %d회 반복 평균 비교 횟수\n", trials);
    printf("%8s %16s %14s %14s %14s %14s\n", "n", "Insertion", "Shell(Orig)", "Shell(Knuth)", "Ins/Orig", "Ins/Knuth");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d %16.1f %14.1f %14.1f %14.1f %14.1f\n", SIZES[s], avg[s][INS], avg[s][SHO], avg[s][SHK],
            avg[s][INS] / avg[s][SHO], avg[s][INS] / avg[s][SHK]);
    printf("\n비교 횟수 / n  (n 이 커질 때 이 값이 얼마나 커지는지)\n");
    printf("%8s %14s %14s %14s\n", "n", "Insertion", "Shell(Orig)", "Shell(Knuth)");
    for (int s = 0; s < NSIZES; s++)
        printf("%8d %14.2f %14.2f %14.2f\n", SIZES[s], avg[s][INS] / SIZES[s], avg[s][SHO] / SIZES[s], avg[s][SHK] / SIZES[s]);

    /* ---------------- 추가 분석 3 : 증가 지수 (비교 ~ n^p 라고 보았을 때의 p) ---------------- */
    printf("\n비교 횟수가 n^p 에 비례한다고 볼 때의 지수 p  (인접한 두 크기 사이)\n");
    printf("%14s %12s %14s %14s\n", "구간", "Insertion", "Shell(Orig)", "Shell(Knuth)");
    for (int s = 1; s < NSIZES; s++) {
        double dn = log((double)SIZES[s] / SIZES[s - 1]);
        printf("%6d->%-6d %12.2f %14.2f %14.2f\n", SIZES[s - 1], SIZES[s],
            log(avg[s][INS] / avg[s - 1][INS]) / dn,
            log(avg[s][SHO] / avg[s - 1][SHO]) / dn,
            log(avg[s][SHK] / avg[s - 1][SHK]) / dn);
    }

    /* ---------------- 추가 분석 4 : 단계 수와 단계당 최소 비용 ----------------
     * 간격 g 의 단계는 원소마다 최소 1번은 비교해야 하므로 적어도 (n - g) 번이다.
     * 따라서 쉘 정렬의 비교 횟수 = Σ(n - g)  [단계 수가 만드는 고정 비용]  +  나머지 [실제로 밀어낸 횟수 등] */
    printf("\n[추가 분석 4] 쉘 정렬 비교 횟수 = 단계별 최소 비용 합 Σ(n-g) + 나머지  (%d회 평균)\n", trials);
    printf("%8s | %5s %10s %10s | %5s %10s %10s\n", "n", "단계", "Σ(n-g)", "나머지", "단계", "Σ(n-g)", "나머지");
    printf("%8s | %27s | %27s\n", "", "Original", "Knuth");
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s], g[MAX_GAPS];
        int co = gaps_original(n, g); long long bo = 0; for (int i = 0; i < co; i++) bo += n - g[i];
        int ck = gaps_knuth(n, g);    long long bk = 0; for (int i = 0; i < ck; i++) bk += n - g[i];
        printf("%8d | %5d %10lld %10.1f | %5d %10lld %10.1f\n", n, co, bo, avg[s][SHO] - bo, ck, bk, avg[s][SHK] - bk);
    }

    printf("\n[자체 검증] 정렬 실행 %d회: 오름차순 위반 %d, 데이터 유실/변형 %d, 삽입 정렬 이론값 불일치 %d\n",
        runs, bad_sorted, bad_multiset, bad_theory);
    printf("[시드] %llu  (1회 실험은 이 시드의 첫 데이터, 평균 실험은 이어지는 난수 사용)\n", (unsigned long long)seed);
    return (bad_sorted || bad_multiset || bad_theory) ? 1 : 0;
}
/* ==========================================================================
 *  main.c  —  과제-10 : 정렬 알고리즘의 성능 및 안정성 비교
 *
 *  사용법 : ./sort_compare [seed] [trials] [partial_swaps]
 *           기본값  seed=20251022  trials=100  partial_swaps=500
 *  출력은 과제 8장 형식을 따르고, 맨 끝에 자체 검증 결과를 덧붙인다.
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "A10_Data.h"

#define N 10000
enum { SEL, BUB, BUBE, INSA, INSB, NALG };      /* 측정 대상 5종 */

static const char* ALG_NAME[NALG] = {
    "Selection Sort", "Bubble Sort", "Bubble Sort (Early Termination)",
    "Insertion Sort (Case A: >)", "Insertion Sort (Case B: >=)"
};
typedef void (*SortFn)(Record*, int, SortStats*);
static const SortFn ALG_FN[NALG] = {
    selection_sort, bubble_sort, bubble_sort_early, insertion_sort_gt, insertion_sort_ge
};

static int bad_sorted = 0, bad_multiset = 0, bad_theory = 0, runs_checked = 0;

static SortStats expected_of(const Expected* e, int alg)
{
    switch (alg) {
    case SEL:  return e->sel;
    case BUB:  return e->bub;
    case BUBE: return e->bub_early;
    case INSA: return e->ins_gt;
    default:   return e->ins_ge;
    }
}

/* input 의 복사본에 alg 번 정렬을 수행하고 결과를 검증한다. 정렬된 결과는 out 에 남긴다. */
static SortStats run_one(int alg, const Record* input, Record* out, const Expected* exp)
{
    SortStats st;
    memcpy(out, input, sizeof(Record) * N);          /* 원본 복사(횟수에 미포함) */
    ALG_FN[alg](out, N, &st);
    runs_checked++;
    if (!is_sorted_key1(out, N))        bad_sorted++;
    if (!same_multiset(out, input, N))  bad_multiset++;
    SortStats ex = expected_of(exp, alg);
    if (ex.cmp != st.cmp || ex.mov != st.mov) {
        bad_theory++;
        fprintf(stderr, "[이론값 불일치] %s 측정 %lld/%lld 이론 %lld/%lld\n",
            ALG_NAME[alg], st.cmp, st.mov, ex.cmp, ex.mov);
    }
    return st;
}

static void print_state(const char* title, const Record* input)
{
    static Record out[N];
    Expected exp;
    theory_expected(input, N, &exp);
    printf("%s\n\n", title);
    for (int a = 0; a < NALG; a++) {
        SortStats st = run_one(a, input, out, &exp);
        printf("%s\nComparisons : %lld\nMovements   : %lld\n\n", ALG_NAME[a], st.cmp, st.mov);
    }
}

/* 안정성 검증 : 한 시험 데이터에 대해 4가지 정렬의 역전된 동일 키 쌍 수를 구한다 */
static void stability_on(const Record* input, int n, long viol[4])
{
    static Record out[N];
    SortFn fns[4] = { selection_sort, bubble_sort, insertion_sort_gt, insertion_sort_ge };
    SortStats st;
    for (int k = 0; k < 4; k++) {
        memcpy(out, input, sizeof(Record) * n);
        fns[k](out, n, &st);
        if (!is_sorted_key1(out, n)) bad_sorted++;
        viol[k] = stability_violations(out, n);
    }
}

int main(int argc, char** argv)
{
    uint64_t seed = argc > 1 ? strtoull(argv[1], NULL, 10) : 20251022ULL;
    int trials = argc > 2 ? atoi(argv[2]) : 100;
    int swaps = argc > 3 ? atoi(argv[3]) : 500;
    if (trials < 1) trials = 1;
    rng_seed(seed);

    static Record original[N], data[N], out[N];
    static Record sorted_in[N], reverse_in[N], partial_in[N];
    long long tot_cmp[NALG] = { 0 }, tot_mov[NALG] = { 0 };

    /* ---------------- 4. 무작위 데이터 반복 실험 ---------------- */
    /* 진행 상황은 stderr 로 출력한다 (결과 출력 stdout 과 섞이지 않고, 파일로 저장해도 깨끗하다) */
    fprintf(stderr, "실험 시작: 무작위 %d회 (약 30초 걸립니다. -O2 없이 컴파일하면 더 오래 걸립니다)\n", trials);
    for (int t = 0; t < trials; t++) {
        if (t % 10 == 0) { fprintf(stderr, "  진행 %d / %d\n", t, trials); fflush(stderr); }
        gen_random(data, N);
        if (t == 0) memcpy(original, data, sizeof data);   /* 1회차 데이터를 입력 상태별 실험의 "원본"으로 사용 */
        Expected exp;
        theory_expected(data, N, &exp);
        for (int a = 0; a < NALG; a++) {                    /* 모든 정렬이 같은 data 의 복사본을 받는다 */
            SortStats st = run_one(a, data, out, &exp);
            tot_cmp[a] += st.cmp;
            tot_mov[a] += st.mov;
        }
    }

    fprintf(stderr, "  진행 %d / %d 완료. 입력 상태별 실험 중...\n\n", trials, trials);
    fflush(stderr);
    printf("Data Count : %d\n\n", N);
    printf("Random Data\nNumber of Trials : %d\n\n", trials);
    for (int a = 0; a < NALG; a++) {
        printf("%s\n", ALG_NAME[a]);
        printf("Total Comparisons   : %lld\n", tot_cmp[a]);
        printf("Average Comparisons : %.1f\n", (double)tot_cmp[a] / trials);
        printf("Average Movements   : %.1f\n\n", (double)tot_mov[a] / trials);
    }
    printf("------------------------------------\n\n");

    /* ---------------- 7. 입력 상태별 (같은 원본에서 생성) ---------------- */
    make_sorted(sorted_in, original, N);
    make_reverse(reverse_in, original, N);
    make_partial(partial_in, original, N, swaps);

    print_state("Sorted Data", sorted_in);
    printf("------------------------------------\n\n");
    print_state("Reverse-Sorted Data", reverse_in);
    printf("------------------------------------\n\n");
    print_state("Partially Sorted Data", partial_in);
    printf("------------------------------------\n\n");

    /* ---------------- 6. 안정성 검증 ---------------- */
    static Record dup[N], eq[N];
    Record small[4] = { {527,100,0}, {527,200,1}, {125,300,2}, {892,400,3} };   /* 과제 예시 */
    gen_duplicates(dup, N, 99);                         /* key1 0..99 -> 키마다 평균 100개 중복 */
    for (int i = 0; i < 1000; i++) { eq[i].key1 = 7; eq[i].key2 = i; eq[i].orig = i; }  /* 전부 같은 키 */

    const char* set_name[4] = { "과제 예시 4개", "무작위 10,000개 (Key1 0~1,000,000)",
                                "중복 시험 10,000개 (Key1 0~99)", "전부 같은 키 1,000개" };
    long viol[4][4];
    stability_on(small, 4, viol[0]);
    stability_on(original, N, viol[1]);
    stability_on(dup, N, viol[2]);
    stability_on(eq, 1000, viol[3]);

    const char* stab_name[4] = { "Selection Sort      ", "Bubble Sort         ",
                                 "Insertion Sort (>)  ", "Insertion Sort (>=) " };
    printf("Stability Verification\n\n");
    for (int k = 0; k < 4; k++) {
        long total = 0;
        for (int s = 0; s < 4; s++) total += viol[s][k];
        if (total == 0) printf("%s: Stable (순서가 뒤집힌 동일 키 쌍 0개)\n", stab_name[k]);
        else            printf("%s: NOT Stable (순서가 뒤집힌 동일 키 쌍 발견)\n", stab_name[k]);
    }
    printf("\n[시험 데이터별 순서가 뒤집힌 동일 키 인접 쌍 수]\n");
    printf("%-38s %10s %8s %8s %8s\n", "시험 데이터", "Selection", "Bubble", "Ins(>)", "Ins(>=)");
    for (int s = 0; s < 4; s++)
        printf("%-38s %10ld %8ld %8ld %8ld\n", set_name[s], viol[s][0], viol[s][1], viol[s][2], viol[s][3]);

    /* 과제 예시 4개 데이터의 정렬 전후 (key1,key2,orig) */
    printf("\n[과제 예시 데이터 정렬 결과: (Key1,Key2)#원래순번]\n");
    SortFn fns[4] = { selection_sort, bubble_sort, insertion_sort_gt, insertion_sort_ge };
    for (int k = 0; k < 4; k++) {
        Record w[4]; SortStats st;
        memcpy(w, small, sizeof w);
        fns[k](w, 4, &st);
        printf("%s:", stab_name[k]);
        for (int i = 0; i < 4; i++) printf(" (%d,%d)#%d", w[i].key1, w[i].key2, w[i].orig);
        printf("\n");
    }

    /* ---------------- 추가 분석 : 선택 정렬 교환 생략 변형 ---------------- */
    printf("\n------------------------------------\n\n[추가 분석] 선택 정렬 변형 (이미 제자리면 교환 생략)\n");
    const Record* ins[4] = { original, sorted_in, reverse_in, partial_in };
    const char* ins_name[4] = { "Random(1회차)", "Sorted", "Reverse-Sorted", "Partially Sorted" };
    for (int s = 0; s < 4; s++) {
        SortStats st;
        memcpy(out, ins[s], sizeof out);
        selection_sort_skip(out, N, &st);
        if (!is_sorted_key1(out, N)) bad_sorted++;
        printf("%-18s Comparisons : %lld  Movements : %lld\n", ins_name[s], st.cmp, st.mov);
    }

    /* ---------------- 자체 검증 요약 ---------------- */
    printf("\n[자체 검증] 정렬 실행 %d회: 오름차순 위반 %d, 데이터 유실/변형 %d, 이론값 불일치 %d\n",
        runs_checked, bad_sorted, bad_multiset, bad_theory);
    printf("[부분 정렬 데이터] 정렬된 배열에서 서로 다른 위치 %d쌍을 무작위로 교환 (seed=%llu)\n",
        swaps, (unsigned long long)seed);
    return (bad_sorted || bad_multiset || bad_theory) ? 1 : 0;
}
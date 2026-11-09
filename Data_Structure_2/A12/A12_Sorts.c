#include "A12_Sorts.h"

const char* pivot_name(PivotMethod m)
{
    switch (m) {
    case PIVOT_FIRST:  return "First";
    case PIVOT_RANDOM: return "Random";
    default:           return "Median-3";
    }
}

static void swap_int(int* x, int* y) { int t = *x; *x = *y; *y = t; }

/* ---------------------------------------------------------------- 피벗 선택
 * 구간 [lo, hi] 에서 피벗이 될 원소의 "위치"를 돌려준다. 이 부분만 방법마다 다르다. */
static int pick_pivot(const int* a, int lo, int hi, PivotMethod m, Rng* r, QSStats* st)
{
    switch (m) {
    case PIVOT_FIRST:
        return lo;
    case PIVOT_RANDOM:
        return rng_range(r, lo, hi);               /* 매 분할마다 새로 뽑는다 (난수 생성은 비교에 미포함) */
    default: {
        if (hi - lo + 1 < 3) return lo;            /* 원소가 2개면 가운데가 첫 원소와 같아 중앙값이 의미 없음 */
        int mid = lo + (hi - lo) / 2;
        int x = a[lo], y = a[mid], z = a[hi];
        st->cmp++;                                 /* 세 값의 중앙값을 찾기 위한 값 비교 (2~3회) */
        if (x < y) {
            st->cmp++;
            if (y < z) return mid;                 /* x < y < z */
            st->cmp++;
            return (x < z) ? hi : lo;              /* z <= y 이므로 중앙값은 max(x, z) */
        }
        else {
            st->cmp++;
            if (x < z) return lo;                  /* y <= x < z */
            st->cmp++;
            return (y < z) ? hi : mid;             /* z <= x 이므로 중앙값은 max(y, z) */
        }
    }
    }
}

/* ---------------------------------------------------------------- 분할 (세 방법 공통)
 * 피벗이 a[lo] 에 와 있다고 가정한다. 양쪽에서 좁혀 오는 Hoare 방식 :
 *   i 는 피벗보다 "작은" 동안 오른쪽으로, j 는 피벗보다 "큰" 동안 왼쪽으로. 멈추면 교환.
 *   같은 값에서도 멈추므로 중복 값이 많아도 구간이 균형 있게 나뉜다.
 * 값 비교 a[i] < v, v < a[j] 를 실제로 할 때마다 정확히 1회씩 센다. 피벗의 최종 위치를 돌려준다. */
static int partition(int* a, int lo, int hi, QSStats* st)
{
    int v = a[lo];
    int i = lo, j = hi + 1;
    for (;;) {
        for (;;) {
            i++;
            st->cmp++;
            if (!(a[i] < v)) break;
            if (i == hi) break;                    /* 인덱스 검사 : 비교로 세지 않음 */
        }
        for (;;) {
            j--;
            st->cmp++;
            if (!(v < a[j])) break;
            if (j == lo) break;
        }
        if (i >= j) break;
        swap_int(&a[i], &a[j]);
    }
    swap_int(&a[lo], &a[j]);                       /* 피벗을 최종 위치로 */
    return j;
}

/* 한 구간을 분할하고 통계를 기록한다 (재귀/반복 공통). 피벗의 최종 위치를 돌려준다. */
static int split(int* a, int lo, int hi, int depth, PivotMethod m, Rng* r, QSStats* st)
{
    int p = pick_pivot(a, lo, hi, m, r, st);
    swap_int(&a[lo], &a[p]);                       /* 피벗을 맨 앞으로 (이동은 비교에 미포함) */
    int j = partition(a, lo, hi, st);

    int size = hi - lo + 1;
    st->partitions++;
    if (depth > st->max_depth) st->max_depth = depth;
    if (size >= 64) {
        int left = j - lo, right = hi - j;
        st->big_parts++;
        st->big_balance += (double)(left < right ? left : right) / (size - 1);
    }
    return j;
}

/* ---------------------------------------------------------------- 재귀 버전 */
static void qs_rec(int* a, int lo, int hi, int depth, PivotMethod m, Rng* r, QSStats* st)
{
    if (lo >= hi) return;
    int j = split(a, lo, hi, depth, m, r, st);
    qs_rec(a, lo, j - 1, depth + 1, m, r, st);     /* 왼쪽 먼저 */
    qs_rec(a, j + 1, hi, depth + 1, m, r, st);
}

void quick_sort_recursive(int* a, int n, PivotMethod m, Rng* pivot_rng, QSStats* st)
{
    st->cmp = st->partitions = st->big_parts = 0;
    st->max_depth = 0;
    st->big_balance = 0;
    qs_rec(a, 0, n - 1, 1, m, pivot_rng, st);
}

/* ---------------------------------------------------------------- 반복 버전 (명시적 스택)
 * 재귀와 "구간을 처리하는 순서"가 완전히 같도록 오른쪽을 먼저 넣고 왼쪽을 나중에 넣는다
 * (나중에 넣은 것이 먼저 나온다). 그러면 무작위 피벗도 같은 순서로 뽑혀 비교 횟수가 정확히 같아진다. */
void quick_sort_iterative(int* a, int n, PivotMethod m, Rng* pivot_rng, QSStats* st, Range* stack)
{
    st->cmp = st->partitions = st->big_parts = 0;
    st->max_depth = 0;
    st->big_balance = 0;
    int sp = 0;
    if (n >= 2) { stack[sp].lo = 0; stack[sp].hi = n - 1; stack[sp].depth = 1; sp++; }
    while (sp > 0) {
        sp--;
        int lo = stack[sp].lo, hi = stack[sp].hi, d = stack[sp].depth;
        int j = split(a, lo, hi, d, m, pivot_rng, st);
        if (j + 1 < hi) { stack[sp].lo = j + 1; stack[sp].hi = hi;    stack[sp].depth = d + 1; sp++; }  /* 오른쪽 */
        if (lo < j - 1) { stack[sp].lo = lo;    stack[sp].hi = j - 1; stack[sp].depth = d + 1; sp++; }  /* 왼쪽 */
    }
}
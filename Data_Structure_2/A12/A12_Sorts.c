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

static int pick_pivot(const int* a, int lo, int hi, PivotMethod m, Rng* r, QSStats* st)
{
    switch (m) {
    case PIVOT_FIRST:
        return lo;
    case PIVOT_RANDOM:
        return rng_range(r, lo, hi);               
    default: {
        if (hi - lo + 1 < 3) return lo;            
        int mid = lo + (hi - lo) / 2;
        int x = a[lo], y = a[mid], z = a[hi];
        st->cmp++;                                 
        if (x < y) {
            st->cmp++;
            if (y < z) return mid;                 
            st->cmp++;
            return (x < z) ? hi : lo;              
        }
        else {
            st->cmp++;
            if (x < z) return lo;                  
            st->cmp++;
            return (y < z) ? hi : mid;             
        }
    }
    }
}

static int partition(int* a, int lo, int hi, QSStats* st)
{
    int v = a[lo];
    int i = lo, j = hi + 1;
    for (;;) {
        for (;;) {
            i++;
            st->cmp++;
            if (!(a[i] < v)) break;
            if (i == hi) break;                    
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
    swap_int(&a[lo], &a[j]);                       
    return j;
}

static int split(int* a, int lo, int hi, int depth, PivotMethod m, Rng* r, QSStats* st)
{
    int p = pick_pivot(a, lo, hi, m, r, st);
    swap_int(&a[lo], &a[p]);                       
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

static void qs_rec(int* a, int lo, int hi, int depth, PivotMethod m, Rng* r, QSStats* st)
{
    if (lo >= hi) return;
    int j = split(a, lo, hi, depth, m, r, st);
    qs_rec(a, lo, j - 1, depth + 1, m, r, st);     
    qs_rec(a, j + 1, hi, depth + 1, m, r, st);
}

void quick_sort_recursive(int* a, int n, PivotMethod m, Rng* pivot_rng, QSStats* st)
{
    st->cmp = st->partitions = st->big_parts = 0;
    st->max_depth = 0;
    st->big_balance = 0;
    qs_rec(a, 0, n - 1, 1, m, pivot_rng, st);
}

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
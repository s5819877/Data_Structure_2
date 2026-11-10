#include <stdio.h>
#include <stdlib.h>
#include "A13_Sorts.h"

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

void stats_init(SortStats* st, int probe_stack)
{
    st->cmp = 0; st->max_depth = 0;
    st->heap_bytes = 0; st->heap_now = 0;
    st->probe_stack = probe_stack; st->stack_base = 0; st->stack_bytes = 0;
}

const char* pivot_name(PivotMethod m)
{
    switch (m) {
    case PIVOT_FIRST:  return "First";
    case PIVOT_RANDOM: return "Random";
    default:           return "Median-3";
    }
}

unsigned long long stack_model_bytes(const SortStats* st)
{
    return (unsigned long long)(st->max_depth + 1) * FRAME_BYTES_MODEL;
}
unsigned long long memory_model_bytes(const SortStats* st)
{
    return (unsigned long long)st->heap_bytes + stack_model_bytes(st);
}

static void* tracked_alloc(size_t bytes, SortStats* st)
{
    void* p = malloc(bytes);
    if (!p) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    st->heap_now += bytes;
    if (st->heap_now > st->heap_bytes) st->heap_bytes = st->heap_now;
    return p;
}
static void tracked_free(void* p, size_t bytes, SortStats* st)
{
    free(p);
    st->heap_now -= bytes;
}

static NOINLINE uintptr_t current_sp(void)
{
    volatile char c = 0;
    return (uintptr_t)&c;
}
static void probe(SortStats* st)
{
    uintptr_t sp = current_sp();
    uintptr_t used = st->stack_base > sp ? st->stack_base - sp : sp - st->stack_base;   
    if (used > st->stack_bytes) st->stack_bytes = (size_t)used;
}

static void swap_int(int* x, int* y) { int t = *x; *x = *y; *y = t; }

static void ms_rec(int* a, int* tmp, int lo, int hi, int depth, SortStats* st)
{
    if (st->probe_stack) probe(st);
    if (lo >= hi) return;
    if (depth > st->max_depth) st->max_depth = depth;

    int mid = lo + (hi - lo) / 2;
    ms_rec(a, tmp, lo, mid, depth + 1, st);
    ms_rec(a, tmp, mid + 1, hi, depth + 1, st);

    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
        st->cmp++;                                  
        if (a[i] <= a[j]) tmp[k++] = a[i++];        
        else              tmp[k++] = a[j++];
    }
    while (i <= mid) tmp[k++] = a[i++];            
    while (j <= hi)  tmp[k++] = a[j++];
    for (k = lo; k <= hi; k++) a[k] = tmp[k];
}

void merge_sort(int* a, int n, SortStats* st)
{
    int probe_flag = st->probe_stack;
    stats_init(st, probe_flag);
    if (n < 2) return;
    int* tmp = (int*)tracked_alloc(sizeof(int) * (size_t)n, st);   
    if (probe_flag) st->stack_base = current_sp();
    ms_rec(a, tmp, 0, n - 1, 1, st);
    tracked_free(tmp, sizeof(int) * (size_t)n, st);
}

static int pick_pivot(const int* a, int lo, int hi, PivotMethod m, Rng* r, SortStats* st)
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

static int partition(int* a, int lo, int hi, SortStats* st)
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

static void qs_rec(int* a, int lo, int hi, int depth, PivotMethod m, Rng* r, SortStats* st)
{
    if (st->probe_stack) probe(st);
    if (lo >= hi) return;
    if (depth > st->max_depth) st->max_depth = depth;

    int p = pick_pivot(a, lo, hi, m, r, st);
    swap_int(&a[lo], &a[p]);                        
    int j = partition(a, lo, hi, st);
    qs_rec(a, lo, j - 1, depth + 1, m, r, st);
    qs_rec(a, j + 1, hi, depth + 1, m, r, st);
}

void quick_sort(int* a, int n, PivotMethod m, Rng* pivot_rng, SortStats* st)
{
    int probe_flag = st->probe_stack;
    stats_init(st, probe_flag);
    if (probe_flag) st->stack_base = current_sp();
    qs_rec(a, 0, n - 1, 1, m, pivot_rng, st);        /* 퀵 정렬은 추가 배열을 잡지 않는다 */
}

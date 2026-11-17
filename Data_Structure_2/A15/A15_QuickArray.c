#include "A15_QuickArray.h"

static void swap_int(int* x, int* y, Counters* c)
{
    int t = *x; *x = *y; *y = t;
    c->mov += 3;                                     
}

static int median3(const int* a, int lo, int hi, Counters* c)
{
    if (hi - lo + 1 < 3) return lo;
    int mid = lo + (hi - lo) / 2;
    int x = a[lo], y = a[mid], z = a[hi];
    c->cmp++;
    if (x < y) {
        c->cmp++;
        if (y < z) return mid;
        c->cmp++;
        return (x < z) ? hi : lo;
    }
    else {
        c->cmp++;
        if (x < z) return lo;
        c->cmp++;
        return (y < z) ? hi : mid;
    }
}

static int partition(int* a, int lo, int hi, Counters* c)
{
    int v = a[lo];
    int i = lo, j = hi + 1;
    for (;;) {
        for (;;) { i++; c->cmp++; if (!(a[i] < v)) break; if (i == hi) break; }
        for (;;) { j--; c->cmp++; if (!(v < a[j])) break; if (j == lo) break; }
        if (i >= j) break;
        swap_int(&a[i], &a[j], c);
    }
    swap_int(&a[lo], &a[j], c);
    return j;
}

static void qs_rec(int* a, int lo, int hi, int depth, Counters* c)
{
    if (lo >= hi) return;
    if (depth > c->max_depth) c->max_depth = depth;
    int p = median3(a, lo, hi, c);
    swap_int(&a[lo], &a[p], c);                      
    int j = partition(a, lo, hi, c);
    qs_rec(a, lo, j - 1, depth + 1, c);
    qs_rec(a, j + 1, hi, depth + 1, c);
}

void quick_sort(int* a, int n, Counters* c) { qs_rec(a, 0, n - 1, 1, c); }

static int scan(const int* a, int n, int v, Counters* c, int* equal)
{
    int i = 0;
    *equal = 0;
    while (i < n) {
        c->cmp++;
        int d = (a[i] > v) - (a[i] < v);             
        if (d >= 0) { *equal = (d == 0); break; }
        i++;
    }
    return i;
}

void arr_insert(int* a, int* n, int v, Counters* c)
{
    int eq;
    int pos = scan(a, *n, v, c, &eq);
    for (int j = *n; j > pos; j--) { a[j] = a[j - 1]; c->mov++; }
    a[pos] = v; c->mov++;
    (*n)++;
}

int arr_delete(int* a, int* n, int v, Counters* c)
{
    int eq;
    int pos = scan(a, *n, v, c, &eq);
    if (pos >= *n || !eq) return 0;
    for (int j = pos; j < *n - 1; j++) { a[j] = a[j + 1]; c->mov++; }
    (*n)--;
    return 1;
}
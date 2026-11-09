#ifndef A12_SORTS_H
#define A12_SORTS_H
#include "A12_Util.h"

typedef enum { PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3 } PivotMethod;

typedef struct {
    long long cmp;         
    long long partitions;  
    int       max_depth;   
    long long big_parts;   
    double    big_balance; 
} QSStats;

typedef struct { int lo, hi, depth; } Range;      

const char* pivot_name(PivotMethod m);

void quick_sort_recursive(int* a, int n, PivotMethod m, Rng* pivot_rng, QSStats* st);

void quick_sort_iterative(int* a, int n, PivotMethod m, Rng* pivot_rng, QSStats* st, Range* stack);

#endif
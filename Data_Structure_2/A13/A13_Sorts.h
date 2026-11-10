#ifndef A13_SORTS_H
#define A13_SORTS_H
#include <stddef.h>
#include <stdint.h>
#include "A13_Util.h"

#define FRAME_BYTES_MODEL 64      

typedef enum { PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3 } PivotMethod;

typedef struct {
    long long cmp;          
    int       max_depth;    
    size_t    heap_bytes;   
    size_t    heap_now;     
    int       probe_stack;  
    uintptr_t stack_base;   
    size_t    stack_bytes;  
} SortStats;

void stats_init(SortStats* st, int probe_stack);        
const char* pivot_name(PivotMethod m);

void merge_sort(int* a, int n, SortStats* st);
void quick_sort(int* a, int n, PivotMethod m, Rng* pivot_rng, SortStats* st);

unsigned long long memory_model_bytes(const SortStats* st);
unsigned long long stack_model_bytes(const SortStats* st);

#endif

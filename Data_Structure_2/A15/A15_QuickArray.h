#ifndef A15_QUICKARRAY_H
#define A15_QUICKARRAY_H
#include "A15_Counters.h"

void quick_sort(int* a, int n, Counters* c);

void arr_insert(int* a, int* n, int v, Counters* c);

int arr_delete(int* a, int* n, int v, Counters* c);

#endif
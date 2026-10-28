#ifndef A11_SORTS_H
#define A11_SORTS_H

#define MAX_GAPS 64

int gaps_original(int n, int* gaps);   
int gaps_knuth(int n, int* gaps);      

long long insertion_sort(int* a, int n);
long long shell_sort_original(int* a, int n);
long long shell_sort_knuth(int* a, int n);

#endif
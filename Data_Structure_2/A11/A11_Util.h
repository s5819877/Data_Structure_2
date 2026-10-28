#ifndef A11_UTIL_H
#define A11_UTIL_H
#include <stdint.h>

#define VALUE_MAX 1000000

void     rng_seed(uint64_t seed);
uint32_t rng_next(void);
int      rng_range(int lo, int hi);            

void gen_random(int* a, int n);                
int  is_sorted(const int* a, int n);           
int  same_multiset(const int* a, const int* b, int n);  

long long insertion_cmp_expected(const int* a, int n);
#endif
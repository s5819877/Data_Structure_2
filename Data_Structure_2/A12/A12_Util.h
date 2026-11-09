#ifndef A12_UTIL_H
#define A12_UTIL_H
#include <stdint.h>

#define VALUE_MAX 1000000

typedef struct { uint64_t s; } Rng;
void     rng_seed(Rng* r, uint64_t seed);
uint32_t rng_next(Rng* r);
int      rng_range(Rng* r, int lo, int hi);       

void gen_random(Rng* r, int* a, int n);           
void gen_sorted(const int* src, int* dst, int n);       
void gen_reverse(const int* src, int* dst, int n);      
int  is_sorted(const int* a, int n);
int  same_multiset(const int* a, const int* b, int n);  
#endif
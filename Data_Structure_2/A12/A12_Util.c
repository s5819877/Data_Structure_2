#include <stdlib.h>
#include <string.h>
#include "A12_Util.h"

void rng_seed(Rng* r, uint64_t seed)
{
    r->s = seed ? seed : 88172645463325252ULL;
    for (int i = 0; i < 8; i++) rng_next(r);
}

uint32_t rng_next(Rng* r)
{
    uint64_t x = r->s;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    r->s = x;
    return (uint32_t)((x * 2685821657736338717ULL) >> 32);
}

int rng_range(Rng* r, int lo, int hi)
{
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    uint32_t limit = 0xFFFFFFFFu - (0xFFFFFFFFu % span) - 1u;
    uint32_t v;
    do { v = rng_next(r); } while (v > limit);
    return lo + (int)(v % span);
}

void gen_random(Rng* r, int* a, int n) { for (int i = 0; i < n; i++) a[i] = rng_range(r, 0, VALUE_MAX); }

static int cmp_up(const void* x, const void* y)
{
    int a = *(const int*)x, b = *(const int*)y;
    return (a > b) - (a < b);
}

/* 입력 준비용 정렬 : 정렬 알고리즘 실험과 무관 (qsort 사용, 비교 횟수 미집계) */
void gen_sorted(const int* src, int* dst, int n)
{
    memcpy(dst, src, sizeof(int) * n);
    qsort(dst, (size_t)n, sizeof(int), cmp_up);
}

void gen_reverse(const int* src, int* dst, int n)
{
    gen_sorted(src, dst, n);
    for (int i = 0; i < n / 2; i++) { int t = dst[i]; dst[i] = dst[n - 1 - i]; dst[n - 1 - i] = t; }
}

int is_sorted(const int* a, int n)
{
    for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

int same_multiset(const int* a, const int* b, int n)
{
    int* cnt = calloc(VALUE_MAX + 1, sizeof(int));
    int ok = 1;
    for (int i = 0; i < n; i++) cnt[a[i]]++;
    for (int i = 0; i < n; i++) if (--cnt[b[i]] < 0) { ok = 0; break; }
    free(cnt);
    return ok;
}
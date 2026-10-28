#include <stdlib.h>
#include <string.h>
#include "A11_Util.h"

static uint64_t st = 88172645463325252ULL;

void rng_seed(uint64_t seed)
{
    st = seed ? seed : 88172645463325252ULL;
    for (int i = 0; i < 8; i++) rng_next();
}

uint32_t rng_next(void)
{
    uint64_t x = st;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    st = x;
    return (uint32_t)((x * 2685821657736338717ULL) >> 32);
}

int rng_range(int lo, int hi)
{
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    uint32_t limit = 0xFFFFFFFFu - (0xFFFFFFFFu % span) - 1u;
    uint32_t r;
    do { r = rng_next(); } while (r > limit);
    return lo + (int)(r % span);
}

void gen_random(int* a, int n) { for (int i = 0; i < n; i++) a[i] = rng_range(0, VALUE_MAX); }

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

long long insertion_cmp_expected(const int* a, int n)
{
    long long total = 0;
    for (int i = 1; i < n; i++) {
        int greater = 0;
        for (int j = 0; j < i; j++) if (a[j] > a[i]) greater++;
        total += greater + (greater == i ? 0 : 1);
    }
    return total;
}
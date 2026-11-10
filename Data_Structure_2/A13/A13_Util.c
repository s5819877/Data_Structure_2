#define _POSIX_C_SOURCE 200809L          /* 리눅스에서 clock_gettime 을 쓰기 위함 (다른 헤더보다 먼저) */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "A13_Util.h"
#ifdef _WIN32                            /* Windows(Visual Studio, MinGW)에는 clock_gettime 이 없다 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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

int is_sorted(const int* a, int n)
{
    for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

int same_multiset(const int* a, const int* b, int n)
{
    int* cnt = (int*)calloc(VALUE_MAX + 1, sizeof(int));
    int ok = 1;
    if (!cnt) return 0;
    for (int i = 0; i < n; i++) cnt[a[i]]++;
    for (int i = 0; i < n; i++) if (--cnt[b[i]] < 0) { ok = 0; break; }
    free(cnt);
    return ok;
}

#ifdef _WIN32
double now_ms(void)
{
    static LARGE_INTEGER freq;
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)freq.QuadPart;
}
#else
double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}
#endif
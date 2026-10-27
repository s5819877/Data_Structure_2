#include <stdlib.h>
#include <string.h>
#include "A10_Data.h"

/* ------------------------------------------------------------ 난수 */
static uint64_t rng_state = 88172645463325252ULL;

void rng_seed(uint64_t seed)
{
    rng_state = seed ? seed : 88172645463325252ULL;
    for (int i = 0; i < 8; i++) rng_next();      /* 초기값 섞기 */
}

uint32_t rng_next(void)
{
    uint64_t x = rng_state;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    rng_state = x;
    return (uint32_t)((x * 2685821657736338717ULL) >> 32);
}

int rng_range(int lo, int hi)
{
    uint32_t span = (uint32_t)(hi - lo) + 1u;
    uint32_t limit = 0xFFFFFFFFu - (0xFFFFFFFFu % span) - 1u;   /* 나머지 편향 제거 */
    uint32_t r;
    do { r = rng_next(); } while (r > limit);
    return lo + (int)(r % span);
}

/* ------------------------------------------------------------ 데이터 생성 */
void gen_random(Record* a, int n)
{
    for (int i = 0; i < n; i++) {
        a[i].key1 = rng_range(0, KEY_MAX);
        a[i].key2 = rng_range(0, KEY_MAX);
        a[i].orig = i;
    }
}

void gen_duplicates(Record* a, int n, int maxkey)
{
    for (int i = 0; i < n; i++) {
        a[i].key1 = rng_range(0, maxkey);
        a[i].key2 = rng_range(0, KEY_MAX);
        a[i].orig = i;
    }
}

/* 입력 상태 준비용 정렬 : 검증된 정렬 함수와 무관한 별도의 안정 병합 정렬(비교 횟수 미집계).
 * descending=1 이면 key1 내림차순(같은 키는 원래 순서 유지). */
static void merge_sort(Record* a, Record* tmp, int n, int descending)
{
    if (n < 2) return;
    int mid = n / 2;
    merge_sort(a, tmp, mid, descending);
    merge_sort(a + mid, tmp, n - mid, descending);
    int i = 0, j = mid, k = 0;
    while (i < mid && j < n) {
        int take_left = descending ? (a[i].key1 >= a[j].key1) : (a[i].key1 <= a[j].key1);
        tmp[k++] = take_left ? a[i++] : a[j++];
    }
    while (i < mid) tmp[k++] = a[i++];
    while (j < n)   tmp[k++] = a[j++];
    memcpy(a, tmp, (size_t)n * sizeof(Record));
}

static void renumber(Record* a, int n)
{
    for (int i = 0; i < n; i++) a[i].orig = i;
}

static void arrange(Record* dst, const Record* src, int n, int descending)
{
    Record* tmp = malloc((size_t)n * sizeof(Record));
    memcpy(dst, src, (size_t)n * sizeof(Record));
    merge_sort(dst, tmp, n, descending);
    free(tmp);
    renumber(dst, n);
}

void make_sorted(Record* dst, const Record* src, int n) { arrange(dst, src, n, 0); }
void make_reverse(Record* dst, const Record* src, int n) { arrange(dst, src, n, 1); }

void make_partial(Record* dst, const Record* src, int n, int swaps)
{
    arrange(dst, src, n, 0);
    for (int s = 0; s < swaps; s++) {
        int i = rng_range(0, n - 1), j = rng_range(0, n - 1);
        if (i == j) { s--; continue; }
        Record t = dst[i]; dst[i] = dst[j]; dst[j] = t;
    }
    renumber(dst, n);
}

/* ------------------------------------------------------------ 검증 */
int is_sorted_key1(const Record* a, int n)
{
    for (int i = 1; i < n; i++) if (a[i - 1].key1 > a[i].key1) return 0;
    return 1;
}

int same_multiset(const Record* a, const Record* b, int n)
{
    /* orig 는 입력 순번이라 서로 달라야 하므로, orig 로 원본을 찾아 key1/key2 일치 + orig 중복 없음 확인 */
    char* seen = calloc((size_t)n, 1);
    int ok = 1;
    for (int i = 0; i < n && ok; i++) {
        int o = a[i].orig;
        if (o < 0 || o >= n || seen[o]) { ok = 0; break; }
        seen[o] = 1;
        if (b[o].key1 != a[i].key1 || b[o].key2 != a[i].key2 || b[o].orig != o) ok = 0;
    }
    free(seen);
    return ok;
}

long stability_violations(const Record* a, int n)
{
    long v = 0;
    for (int i = 1; i < n; i++)
        if (a[i - 1].key1 == a[i].key1 && a[i - 1].orig > a[i].orig) v++;
    return v;
}

/* ------------------------------------------------------------ 이론 값
 * 입력 배열 x[0..n-1] 에 대해 각 위치 i 의 g_i = (앞쪽에서 x[i] 보다 "큰" 원소 수),
 * h_i = (앞쪽에서 x[i] 보다 "크거나 같은" 원소 수) 를 펜윅 트리로 구한다.
 *   선택  : 비교 n(n-1)/2, 이동 3(n-1)
 *   버블  : 비교 n(n-1)/2, 이동 3*Σg_i
 *           (한 번의 교환 = 역전 쌍 하나 제거, 같은 키는 교환하지 않으므로 역전은 엄격한 >)
 *   조기  : 순회마다 "가장 앞쪽으로 밀리는 원소"가 한 칸씩 줄어든다 -> 교환이 있는 순회 수 = max g_i,
 *           마지막에 교환 없는 순회 1회 추가 (n-1 이 상한). 순회 p 의 비교는 n-1-p.
 *   삽입A : 이동 2(n-1) + Σg_i, 비교 Σg_i + (n-1) - #{i>=1 : g_i == i}
 *   삽입B : 이동 2(n-1) + Σh_i, 비교 Σh_i + (n-1) - #{i>=1 : h_i == i}
 *           (g_i == i / h_i == i 이면 j<0 으로 끝나 마지막 "실패 비교"가 없다) */
static int fen[KEY_MAX + 2];

static void fen_add(int k) { for (k++; k <= KEY_MAX + 1; k += k & -k) fen[k]++; }
static void fen_sub(int k) { for (k++; k <= KEY_MAX + 1; k += k & -k) fen[k]--; }
static int  fen_cnt_le(int k) { int s = 0; for (k++; k > 0; k -= k & -k) s += fen[k]; return s; }  /* key <= k 개수 */

void theory_expected(const Record* x, int n, Expected* e)
{
    long long sum_g = 0, sum_h = 0, no_fail_g = 0, no_fail_h = 0;
    int max_g = 0;
    for (int i = 0; i < n; i++) {
        int le = fen_cnt_le(x[i].key1);                              /* 앞쪽에서 <= 인 개수 */
        int lt = x[i].key1 > 0 ? fen_cnt_le(x[i].key1 - 1) : 0;      /* 앞쪽에서 <  인 개수 */
        int g = i - le, h = i - lt;
        sum_g += g; sum_h += h;
        if (g > max_g) max_g = g;
        if (i >= 1 && g == i) no_fail_g++;
        if (i >= 1 && h == i) no_fail_h++;
        fen_add(x[i].key1);
    }
    for (int i = 0; i < n; i++) fen_sub(x[i].key1);                  /* 펜윅 비우기 */

    long long N = n, pairs = N * (N - 1) / 2;
    e->sel.cmp = pairs;               e->sel.mov = 3 * (N - 1);
    e->bub.cmp = pairs;               e->bub.mov = 3 * sum_g;
    long long passes = (long long)max_g + 1;
    if (passes > N - 1) passes = N - 1;
    long long c = 0;
    for (long long p = 0; p < passes; p++) c += N - 1 - p;
    e->bub_early.cmp = c;             e->bub_early.mov = 3 * sum_g;
    e->ins_gt.cmp = sum_g + (N - 1) - no_fail_g;   e->ins_gt.mov = 2 * (N - 1) + sum_g;
    e->ins_ge.cmp = sum_h + (N - 1) - no_fail_h;   e->ins_ge.mov = 2 * (N - 1) + sum_h;
}
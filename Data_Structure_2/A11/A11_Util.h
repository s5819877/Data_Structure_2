/* util.h — 난수, 데이터 생성, 정렬 결과 검증, 이론 비교 횟수 (정렬 횟수에 포함되지 않는 보조 기능) */
#ifndef A11_UTIL_H
#define A11_UTIL_H
#include <stdint.h>

#define VALUE_MAX 1000000

void     rng_seed(uint64_t seed);
uint32_t rng_next(void);
int      rng_range(int lo, int hi);            /* lo..hi 포함, 편향 없음 */

void gen_random(int* a, int n);                /* 0..VALUE_MAX 무작위 (중복 허용) */
int  is_sorted(const int* a, int n);           /* 오름차순이면 1 */
int  same_multiset(const int* a, const int* b, int n);  /* 같은 값들의 재배열이면 1 */

/* 삽입 정렬 비교 횟수를 정렬 코드와 무관하게 입력에서 직접 계산 (O(n^2) 이중 반복):
 *   원소 i 마다 (앞쪽에서 a[i] 보다 큰 원소 수 = 이동 횟수) + (앞쪽 전체가 더 크면 0, 아니면 1) */
long long insertion_cmp_expected(const int* a, int n);
#endif
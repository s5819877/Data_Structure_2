/* util.h — 난수, 데이터 생성, 정렬 결과 검증 (정렬 비교 횟수에 포함되지 않는 보조 기능) */
#ifndef A12_UTIL_H
#define A12_UTIL_H
#include <stdint.h>

#define VALUE_MAX 1000000

/* 난수 생성기 (xorshift64*). 데이터 생성용과 피벗 선택용을 따로 두기 위해 구조체로 만들었다.
 * 피벗 선택이 데이터 생성 난수에 영향을 주지 않는다. rand()/RAND_MAX 는 플랫폼마다 달라 쓰지 않는다. */
typedef struct { uint64_t s; } Rng;
void     rng_seed(Rng* r, uint64_t seed);
uint32_t rng_next(Rng* r);
int      rng_range(Rng* r, int lo, int hi);       /* lo..hi 포함, 편향 없음 */

void gen_random(Rng* r, int* a, int n);           /* 0..VALUE_MAX 무작위 (중복 허용) */
void gen_sorted(const int* src, int* dst, int n);       /* 추가 분석용 : 오름차순 배열 */
void gen_reverse(const int* src, int* dst, int n);      /* 추가 분석용 : 내림차순 배열 */
int  is_sorted(const int* a, int n);
int  same_multiset(const int* a, const int* b, int n);  /* 같은 값들의 재배열이면 1 */
#endif
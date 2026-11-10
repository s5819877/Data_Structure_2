/* util.h ? 난수, 데이터 생성, 정렬 결과 검증, 시간 측정 (정렬 비교 횟수에 포함되지 않는 보조 기능) */
#ifndef A13_UTIL_H
#define A13_UTIL_H
#include <stdint.h>

#define VALUE_MAX 1000000

/* 난수 생성기 (xorshift64*). 데이터 생성용과 피벗 선택용을 따로 두려고 구조체로 만들었다.
 * rand()/RAND_MAX 는 플랫폼마다 달라 쓰지 않는다. */
typedef struct { uint64_t s; } Rng;
void     rng_seed(Rng* r, uint64_t seed);
uint32_t rng_next(Rng* r);
int      rng_range(Rng* r, int lo, int hi);       /* lo..hi 포함, 편향 없음 */

void gen_random(Rng* r, int* a, int n);           /* 0..VALUE_MAX 무작위 (중복 허용) */
int  is_sorted(const int* a, int n);
int  same_multiset(const int* a, const int* b, int n);  /* 같은 값들의 재배열이면 1 */

/* 시간이 거꾸로 가지 않는 고해상도 타이머 (밀리초). Windows 와 리눅스/맥 모두 동작한다. */
double now_ms(void);
#endif
/* ==========================================================================
 *  data.h  —  난수, 시험 데이터 생성, 검증 도구 (과제-10)
 *  여기의 함수들은 정렬 "준비/검증" 용이며 비교·이동 횟수에 포함되지 않는다.
 * ========================================================================== */
#ifndef A10_DATA_H
#define A10_DATA_H

#include <stdint.h>
#include "A10_Sorts.h"

#define KEY_MAX 1000000

 /* 이식성을 위해 직접 만든 난수 생성기 (xorshift64*). rand()/RAND_MAX 는 플랫폼마다 범위가 달라 쓰지 않는다. */
void     rng_seed(uint64_t seed);
uint32_t rng_next(void);
int      rng_range(int lo, int hi);          /* lo..hi 포함, 편향 없는 거절 표본 추출 */

/* Key1, Key2 를 0..1,000,000 에서 독립적으로 뽑고 orig = 0..n-1 */
void gen_random(Record* a, int n);
/* key1 범위를 0..maxkey 로 줄여 중복이 많은 시험 데이터 (안정성 검증용) */
void gen_duplicates(Record* a, int n, int maxkey);

/* 입력 상태 만들기 : src 를 복사해 정렬/역순/부분 정렬 상태로 배치한 뒤 orig 를 새 위치 순번으로 다시 매긴다.
 * (정렬 알고리즘의 입력 순서 = orig 순서 이므로 안정성 검증 기준이 일관된다.) */
void make_sorted(Record* dst, const Record* src, int n);
void make_reverse(Record* dst, const Record* src, int n);
/* 부분 정렬 : 정렬된 배열에서 서로 다른 위치 쌍 swaps 개를 무작위로 맞바꾼다. */
void make_partial(Record* dst, const Record* src, int n, int swaps);

/* 검증 */
int  is_sorted_key1(const Record* a, int n);                 /* key1 오름차순이면 1 */
int  same_multiset(const Record* a, const Record* b, int n); /* 같은 데이터(key1,key2,orig)의 재배열이면 1 */
/* 정렬 결과에서 key1 이 같은 인접 쌍 중 orig 순서가 뒤집힌 쌍의 수. 0 이면 안정 */
long stability_violations(const Record* a, int n);

/* 입력 데이터만으로 계산한 이론 횟수 (정렬을 돌리지 않고 구한 값 -> 측정값과 대조) */
typedef struct { SortStats sel, bub, bub_early, ins_gt, ins_ge; } Expected;
void theory_expected(const Record* input, int n, Expected* e);

#endif
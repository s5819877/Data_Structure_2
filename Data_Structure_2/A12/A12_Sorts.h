/* ==========================================================================
 *  sorts.h  —  피벗 선택 방법별 퀵 정렬 (재귀 / 반복) 과 비교 횟수 측정 (과제-12)
 *
 *  세 방법은 "피벗을 고르는 부분"만 다르고 분할 알고리즘은 완전히 같다.
 *  재귀 버전과 반복 버전도 같은 피벗 선택과 같은 분할 함수를 쓴다.
 *
 *  비교 횟수 기준
 *    - 분할 중 "데이터 값과 피벗 값"을 견주는 연산 1회 = 1 (결과와 무관)
 *    - 중앙값 선택에서 세 값의 대소를 판단하는 값 비교도 센다
 *    - 인덱스/종료 조건 검사(i == hi 등), 피벗 위치를 만드는 난수, 데이터 이동은 세지 않는다
 * ========================================================================== */
#ifndef A12_SORTS_H
#define A12_SORTS_H
#include "A12_Util.h"

typedef enum { PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3 } PivotMethod;

typedef struct {
    long long cmp;         /* 값 비교 횟수 (피벗 선택용 비교 포함) */
    long long partitions;  /* 분할을 수행한 횟수 (크기 2 이상인 구간) */
    int       max_depth;   /* 가장 깊은 분할 단계 (루트 구간 = 1) */
    long long big_parts;   /* 크기 64 이상 구간의 분할 수 */
    double    big_balance; /* 그 분할들의 균형도 합 : min(왼쪽, 오른쪽) / (구간 크기 - 1)  (0.5 가 정확히 절반) */
} QSStats;

typedef struct { int lo, hi, depth; } Range;      /* 반복 버전의 스택 원소 */

const char* pivot_name(PivotMethod m);

/* 모두 오름차순. st 는 함수 안에서 0 으로 초기화된다. pivot_rng 는 PIVOT_RANDOM 에서만 쓰인다. */
void quick_sort_recursive(int* a, int n, PivotMethod m, Rng* pivot_rng, QSStats* st);
/* stack 은 호출자가 미리 마련한 n + 2 개 이상의 Range 배열 (할당 시간을 측정에서 빼기 위함) */
void quick_sort_iterative(int* a, int n, PivotMethod m, Rng* pivot_rng, QSStats* st, Range* stack);

#endif
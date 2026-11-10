/* ==========================================================================
 *  sorts.h  —  병합 정렬과 퀵 정렬(피벗 3종)의 비교 횟수 / 메모리 측정 (과제-13)
 *
 *  비교 횟수 기준
 *    - 병합 정렬 : 병합할 때 두 값을 견주는 연산 1회 = 1
 *    - 퀵 정렬   : 분할 중 "데이터 값과 피벗 값"을 견주는 연산 1회 = 1, 중앙값 선택에서 세 값의 대소를 가리는 비교도 센다
 *    - 인덱스/종료 조건 검사, 난수 생성, 데이터 이동은 세지 않는다
 *
 *  메모리 기준 (원본 배열은 공통 비용이라 제외)
 *    - 추가 배열 : 정렬 중 malloc 으로 잡은 양을 실제로 센다 (병합 정렬의 임시 배열)
 *    - 호출 스택 : (최대 재귀 깊이 + 1) x 호출 1회당 스택 크기.  +1 은 맨 아래 단계에서 바로 돌아오는 호출 한 번.
 *                  호출 1회당 크기는 FRAME_BYTES_MODEL 로 가정한다. 컴파일러마다 달라서,
 *                  실제로 스택을 얼마나 썼는지도 따로 재서(stack_bytes) 비교한다.
 * ========================================================================== */
#ifndef A13_SORTS_H
#define A13_SORTS_H
#include <stddef.h>
#include <stdint.h>
#include "A13_Util.h"

#define FRAME_BYTES_MODEL 64      /* 64비트 환경에서 재귀 호출 1회가 쓰는 스택의 가정값 (반환 주소 + 인자 + 지역 변수 + 정렬 맞춤) */

typedef enum { PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3 } PivotMethod;

typedef struct {
    long long cmp;          /* 값 비교 횟수 */
    int       max_depth;    /* 실제 작업(분할/병합)을 한 재귀 호출의 가장 깊은 단계 (첫 호출 = 1) */
    size_t    heap_bytes;   /* 정렬 중 추가로 할당한 메모리의 최댓값 (Byte) */
    size_t    heap_now;     /* (내부용) 지금 할당 중인 양 */
    int       probe_stack;  /* 입력 : 1 이면 실제 스택 사용량도 잰다 (시간 측정할 때는 0) */
    uintptr_t stack_base;   /* (내부용) */
    size_t    stack_bytes;  /* 출력 : probe_stack 이 1 일 때 실제로 측정한 최대 스택 사용량 (Byte) */
} SortStats;

void stats_init(SortStats* st, int probe_stack);        /* 정렬 호출 전에 한 번 부른다 */
const char* pivot_name(PivotMethod m);

void merge_sort(int* a, int n, SortStats* st);
void quick_sort(int* a, int n, PivotMethod m, Rng* pivot_rng, SortStats* st);

/* 표에 출력하는 메모리 = 추가 배열 + 호출 스택(계산값) */
unsigned long long memory_model_bytes(const SortStats* st);
unsigned long long stack_model_bytes(const SortStats* st);

#endif
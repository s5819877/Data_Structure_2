/* ==========================================================================
 *  sorts.h  —  비교/이동 횟수를 세는 정렬 알고리즘 (과제-10)
 *
 *  하나의 데이터 = Record {key1, key2, orig}
 *    key1 : 정렬 기준 키 (0 ~ 1,000,000)
 *    key2 : 정렬에 쓰지 않는 두 번째 키 (0 ~ 1,000,000)
 *    orig : 정렬 함수에 넘겨진 시점의 원래 입력 순번 (안정성 검증용, 정렬 기준에 쓰지 않음)
 *  정렬 중 세 값은 하나의 데이터로 함께 이동한다.
 *
 *  횟수를 세는 기준 (과제 3장)
 *    비교 : 두 데이터의 key1 을 견줄 때마다 1회. 반복문의 인덱스 검사(j >= 0 등)는 세지 않는다.
 *    이동 : 데이터 하나를 통째로 다른 저장 위치/임시 변수에 복사·대입할 때마다 1회.
 *           두 데이터를 임시 변수로 교환하면 3회.
 *
 *  모든 정렬 함수는 시작할 때 st 를 0 으로 초기화하고, 끝나면 이번 정렬에서 발생한
 *  비교/이동 횟수를 st 에 담는다. 정렬은 key1 기준 오름차순이다.
 * ========================================================================== */

#ifndef A10_SORTS_H
#define A10_SORTS_H

typedef struct { int key1, key2, orig; } Record;
typedef struct { long long cmp, mov; } SortStats;     /* 합계가 21억을 넘을 수 있어 64비트 */

/* 선택 정렬 : 매 바깥 반복마다 최솟값과 현재 위치를 교환한다 (제자리여도 교환 수행 -> 이동 항상 3(n-1)) */
void selection_sort(Record* a, int n, SortStats* st);

/* 선택 정렬 변형 (추가 분석용) : 최솟값이 이미 현재 위치에 있으면 교환을 건너뛴다 */
void selection_sort_skip(Record* a, int n, SortStats* st);

/* 버블 정렬 : 정해진 n-1 번의 순회를 모두 수행한다 */
void bubble_sort(Record* a, int n, SortStats* st);

/* 버블 정렬 + 조기 종료 : 한 순회에서 교환이 한 번도 없으면 종료한다 (그 외는 위와 동일) */
void bubble_sort_early(Record* a, int n, SortStats* st);

/* 삽입 정렬 Case A : 밀어낼 조건이  data[j].key1 >  current.key1  (안정 정렬) */
void insertion_sort_gt(Record* a, int n, SortStats* st);

/* 삽입 정렬 Case B : 밀어낼 조건이  data[j].key1 >= current.key1  (같은 키도 밀어내므로 불안정) */
void insertion_sort_ge(Record* a, int n, SortStats* st);

#endif /* SORTS_H */
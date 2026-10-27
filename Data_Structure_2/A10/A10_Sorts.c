/* ==========================================================================
 *  sorts.c  —  비교/이동 횟수를 세는 정렬 구현 (과제-10)
 *  라이브러리 정렬(qsort 등)은 사용하지 않고 모두 직접 구현했다.
 *  횟수 규칙은 sorts.h 상단 주석 참조.
 * ========================================================================== */

#include "A10_Sorts.h"

 /* 두 데이터를 임시 변수로 교환 : 데이터 이동 3회 */
static void swap_rec(Record* x, Record* y, SortStats* st)
{
    Record tmp = *x;   /* 이동 1 */
    *x = *y;           /* 이동 2 */
    *y = tmp;          /* 이동 3 */
    st->mov += 3;
}

/* ---------------------------------------------------------------- 선택 정렬 */
void selection_sort(Record* a, int n, SortStats* st)
{
    st->cmp = st->mov = 0;
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++) {
            st->cmp++;                          /* key1 비교 1회 */
            if (a[j].key1 < a[min].key1) min = j;
        }
        swap_rec(&a[i], &a[min], st);           /* 제자리여도 교환 수행 : 이동 3 */
    }
}

void selection_sort_skip(Record* a, int n, SortStats* st)
{
    st->cmp = st->mov = 0;
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++) {
            st->cmp++;
            if (a[j].key1 < a[min].key1) min = j;
        }
        if (min != i) swap_rec(&a[i], &a[min], st);   /* 제자리면 교환 생략 */
    }
}

/* ---------------------------------------------------------------- 버블 정렬 */
void bubble_sort(Record* a, int n, SortStats* st)
{
    st->cmp = st->mov = 0;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            st->cmp++;
            if (a[j].key1 > a[j + 1].key1)      /* 같은 키는 교환하지 않음 -> 안정 */
                swap_rec(&a[j], &a[j + 1], st);
        }
    }
}

void bubble_sort_early(Record* a, int n, SortStats* st)
{
    st->cmp = st->mov = 0;
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            st->cmp++;
            if (a[j].key1 > a[j + 1].key1) {
                swap_rec(&a[j], &a[j + 1], st);
                swapped = 1;
            }
        }
        if (!swapped) break;                    /* 교환이 없었으면 이미 정렬 완료 */
    }
}

/* ---------------------------------------------------------------- 삽입 정렬
 * 이동 규칙 : 삽입할 값을 임시 변수에 저장(1) + 뒤로 밀 때마다(1) + 최종 위치에 넣을 때(1)
 * 최종 위치 대입은 제자리여도 수행하므로 항상 센다 (원소당 최소 2회).
 * 비교 규칙 : 밀어낼지 판단하는 key1 비교 1회마다 1. j >= 0 검사는 세지 않는다. */
void insertion_sort_gt(Record* a, int n, SortStats* st)
{
    st->cmp = st->mov = 0;
    for (int i = 1; i < n; i++) {
        Record cur = a[i];  st->mov++;          /* 임시 저장 */
        int j = i - 1;
        while (j >= 0) {
            st->cmp++;
            if (a[j].key1 > cur.key1) {         /* Case A : 엄격히 클 때만 밀어냄 */
                a[j + 1] = a[j];  st->mov++;
                j--;
            }
            else break;
        }
        a[j + 1] = cur;  st->mov++;             /* 최종 위치에 삽입 */
    }
}

void insertion_sort_ge(Record* a, int n, SortStats* st)
{
    st->cmp = st->mov = 0;
    for (int i = 1; i < n; i++) {
        Record cur = a[i];  st->mov++;
        int j = i - 1;
        while (j >= 0) {
            st->cmp++;
            if (a[j].key1 >= cur.key1) {        /* Case B : 같아도 밀어냄 -> 같은 키 순서 뒤집힘 */
                a[j + 1] = a[j];  st->mov++;
                j--;
            }
            else break;
        }
        a[j + 1] = cur;  st->mov++;
    }
}
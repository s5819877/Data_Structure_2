#include "A11_Sorts.h"

int gaps_original(int n, int* gaps)
{
    int cnt = 0;
    for (int g = n / 2; g >= 1; g /= 2) gaps[cnt++] = g;
    if (cnt == 0) gaps[cnt++] = 1;                 
    return cnt;
}

int gaps_knuth(int n, int* gaps)
{
    int asc[MAX_GAPS], cnt = 0;
    long long h = 1;                               
    while (h < n) { asc[cnt++] = (int)h; h = 3 * h + 1; }   
    if (cnt == 0) asc[cnt++] = 1;
    for (int i = 0; i < cnt; i++) gaps[i] = asc[cnt - 1 - i];  
    return cnt;
}

static long long gap_insertion(int* a, int n, int gap)
{
    long long cmp = 0;
    for (int i = gap; i < n; i++) {
        int cur = a[i];
        int j = i - gap;
        while (j >= 0) {
            cmp++;                                 
            if (a[j] > cur) { a[j + gap] = a[j]; j -= gap; }
            else break;
        }
        a[j + gap] = cur;
    }
    return cmp;
}

long long insertion_sort(int* a, int n) { return gap_insertion(a, n, 1); }

static long long shell_with(int* a, int n, const int* gaps, int cnt)
{
    long long cmp = 0;
    for (int k = 0; k < cnt; k++) cmp += gap_insertion(a, n, gaps[k]);
    return cmp;
}

long long shell_sort_original(int* a, int n)
{
    int gaps[MAX_GAPS], cnt = gaps_original(n, gaps);
    return shell_with(a, n, gaps, cnt);
}

long long shell_sort_knuth(int* a, int n)
{
    int gaps[MAX_GAPS], cnt = gaps_knuth(n, gaps);
    return shell_with(a, n, gaps, cnt);
}
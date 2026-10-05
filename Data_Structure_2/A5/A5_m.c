#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "A5_BST.h"

#define DATA_N    100
#define QUERY_N   50
#define RANGE_MAX 1000

static void fmt_comma(long n, char* out, size_t outlen)
{
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%ld", n);
    int len = (int)strlen(tmp), start = (tmp[0] == '-') ? 1 : 0;
    int ndigits = len - start;
    int ncommas = (ndigits - 1) / 3;
    int outlen_needed = len + ncommas + 1;
    if ((int)outlen < outlen_needed) { snprintf(out, outlen, "%ld", n); return; }

    int oi = outlen_needed - 1;
    out[oi--] = '\0';
    int cnt = 0;
    for (int i = len - 1; i >= start; i--) {
        out[oi--] = tmp[i];
        cnt++;
        if (cnt % 3 == 0 && i != start) out[oi--] = ',';
    }
    if (start) out[oi--] = '-';
}

static int rand_int(void) { return rand() % (RANGE_MAX + 1); }

static void gen_unique(int* out, int n)
{
    static unsigned char seen[RANGE_MAX + 1];
    for (int i = 0; i <= RANGE_MAX; i++) seen[i] = 0;

    for (int i = 0; i < n; i++) {
        int v;
        do { v = rand_int(); } while (seen[v]);
        seen[v] = 1;
        out[i] = v;
    }
}

int main(int argc, char* argv[])
{
    unsigned seed = (argc >= 2) ? (unsigned)atoi(argv[1]) : (unsigned)time(NULL);
    srand(seed);
    printf("난수 시드 : %u  (재현하려면 './search_compare %u' 로 실행)\n\n", seed, seed);

    int arr[DATA_N];
    gen_unique(arr, DATA_N);

    BSTNode* root = NULL;
    long build_cmp = 0;
    for (int i = 0; i < DATA_N; i++)
        root = bst_insert(root, arr[i], &build_cmp);

    printf("===== 1. 생성된 데이터 (%d개, 발생 순서) =====\n", DATA_N);
    for (int i = 0; i < DATA_N; i++) {
        printf("%4d%s", arr[i], (i == DATA_N - 1) ? "\n" : ", ");
        if ((i + 1) % 10 == 0) printf("\n");
    }

    int node_cnt, leaf_cnt, height;
    bst_stats(root, &node_cnt, &leaf_cnt, &height);

    printf("\n===== 2. BST 생성 비용 =====\n");
    printf("  삽입된 노드 수        : %d\n", node_cnt);
    printf("  BST 생성 총 비교 횟수 : %ld\n", build_cmp);
    printf("  삽입 1회당 평균 비교  : %.2f\n", (double)build_cmp / DATA_N);
    printf("  완성된 트리 높이      : %d   (이상적인 균형 트리라면 약 %.1f)\n",
        height, log2((double)DATA_N) + 1.0);
    printf("  단말 노드 수          : %d\n", leaf_cnt);

    int query[QUERY_N];
    for (int i = 0; i < QUERY_N; i++) query[i] = rand_int();

    printf("\n===== 3. 탐색 대상 (%d개) =====\n", QUERY_N);
    for (int i = 0; i < QUERY_N; i++) {
        printf("%4d%s", query[i], (i == QUERY_N - 1) ? "\n" : ", ");
        if ((i + 1) % 10 == 0) printf("\n");
    }

    long seq_cmp_arr[QUERY_N], bst_cmp_arr[QUERY_N];
    int  seq_found[QUERY_N], bst_found[QUERY_N];
    long seq_total = 0, bst_total = 0;
    int  success_cnt = 0;

    for (int i = 0; i < QUERY_N; i++) {
        long sc = 0, bc = 0;
        int sf = 0;
        for (int j = 0; j < DATA_N; j++) {
            sc++;
            if (arr[j] == query[i]) { sf = 1; break; }
        }
        int bf = bst_search(root, query[i], &bc);

        seq_cmp_arr[i] = sc;  seq_found[i] = sf;  seq_total += sc;
        bst_cmp_arr[i] = bc;  bst_found[i] = bf;  bst_total += bc;
        if (sf) success_cnt++;

        if (sf != bf) {
            fprintf(stderr, "[경고] 탐색 키 %d 에서 순차 탐색과 BST 탐색의 결과가 다릅니다! "
                "(배열/트리 데이터 불일치 의심)\n", query[i]);
        }
    }

    printf("\n===== 4. 탐색 결과 =====\n");
    printf("%-13s %-15s %14s %13s\n", "탐색 키", "결과", "순차 비교횟수", "BST 비교횟수");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < QUERY_N; i++) {
        printf("%-10d %-8s %14ld %12ld\n",
            query[i], seq_found[i] ? "성공" : "실패",
            seq_cmp_arr[i], bst_cmp_arr[i]);
    }

    printf("\n----- 출력 형식 예시 (첫 번째 탐색 키) -----\n");
    printf("Search Key : %d\n\n", query[0]);
    printf("Sequential Search\n");
    printf("Result      : %s\n", seq_found[0] ? "Found" : "Not Found");
    printf("Comparisons : %ld\n\n", seq_cmp_arr[0]);
    printf("BST Search\n");
    printf("Result      : %s\n", bst_found[0] ? "Found" : "Not Found");
    printf("Comparisons : %ld\n", bst_cmp_arr[0]);

    char buf1[32], buf2[32];
    double seq_avg = (double)seq_total / QUERY_N;
    double bst_avg = (double)bst_total / QUERY_N;

    printf("\n===== 5. 종합 통계 =====\n");
    printf("Number of searches: %d  (탐색 성공 %d건 / 실패 %d건)\n\n",
        QUERY_N, success_cnt, QUERY_N - success_cnt);

    fmt_comma(seq_total, buf1, sizeof(buf1));
    printf("Sequential Search\n");
    printf("Total comparisons   : %s\n", buf1);
    printf("Average comparisons : %.2f\n\n", seq_avg);

    fmt_comma(bst_total, buf2, sizeof(buf2));
    printf("BST Search\n");
    printf("Total comparisons   : %s\n", buf2);
    printf("Average comparisons : %.2f\n\n", bst_avg);

    printf("----- BST 생성 비용까지 포함한 비교 -----\n");
    long bst_total_with_build = bst_total + build_cmp;
    char buf3[32];
    fmt_comma(bst_total_with_build, buf3, sizeof(buf3));
    printf("BST 생성(%ld) + BST 탐색 50회(%ld) = %s\n",
        build_cmp, bst_total, buf3);
    printf("순차 탐색 50회 총 비교 횟수                = %s\n", buf1);
    printf("배수 (순차 탐색 / (생성+탐색 포함 BST))     : %.2f 배\n",
        (double)seq_total / (double)bst_total_with_build);
    printf("배수 (순차 탐색 / BST 탐색만)                : %.2f 배\n",
        (double)seq_total / (double)bst_total);

    bst_free(root);
    return 0;
}
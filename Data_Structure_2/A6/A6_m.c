#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "A6_BST.h"
#include "A6_AVL.h"

#define DRAW_N    100 
#define QUERY_N   50 
#define RANGE_MAX 1000 

static void fmt_comma(long n, char* out, size_t outlen)
{
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%ld", n);
    int len = (int)strlen(tmp), start = (tmp[0] == '-') ? 1 : 0;
    int ndigits = len - start;
    int ncommas = (ndigits - 1) / 3;
    int needed = len + ncommas + 1;
    if ((int)outlen < needed) { snprintf(out, outlen, "%ld", n); return; }

    int oi = needed - 1;
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

static int array_try_insert(int* arr, int* len, int value, long* cmp_count)
{
    long c = 0;
    int found = 0;
    for (int i = 0; i < *len; i++) {
        c++;
        if (arr[i] == value) { found = 1; break; }
    }
    *cmp_count += c;
    if (!found) arr[(*len)++] = value;
    return !found;
}

static int array_search(const int* arr, int len, int target, long* cmp_count)
{
    for (int i = 0; i < len; i++) {
        (*cmp_count)++;
        if (arr[i] == target) return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    unsigned seed = (argc >= 2) ? (unsigned)atoi(argv[1]) : (unsigned)time(NULL);
    srand(seed);
    printf("난수 시드 : %u  (재현하려면 './compare3 %u' 로 실행)\n\n", seed, seed);

    int draws[DRAW_N];
    for (int i = 0; i < DRAW_N; i++) draws[i] = rand_int();

    printf("===== 1. 생성된 100개의 정수 (발생 순서, 중복 가능) =====\n");
    for (int i = 0; i < DRAW_N; i++) {
        printf("%4d%s", draws[i], (i == DRAW_N - 1) ? "\n" : ", ");
        if ((i + 1) % 10 == 0) printf("\n");
    }

    int   arr[DRAW_N];
    int   arr_len = 0;
    long  array_cmp = 0;

    BSTNode* bst_root = NULL;
    long     bst_cmp = 0;

    AVLNode* avl_root = NULL;
    long     avl_cmp = 0;

    int duplicates = 0;
    int mismatch_warn = 0;

    for (int i = 0; i < DRAW_N; i++) {
        int v = draws[i];

        int ins_arr = array_try_insert(arr, &arr_len, v, &array_cmp);

        int ins_bst;
        bst_root = bst_insert(bst_root, v, &bst_cmp, &ins_bst);

        int ins_avl;
        avl_root = avl_insert(avl_root, v, &avl_cmp, &ins_avl);

        if (!ins_arr) duplicates++;

        if (ins_arr != ins_bst || ins_arr != ins_avl) {
            mismatch_warn++;
            fprintf(stderr, "[경고] 값 %d 에서 세 자료구조의 삽입 여부가 다릅니다! "
                "(array=%d, bst=%d, avl=%d)\n", v, ins_arr, ins_bst, ins_avl);
        }
    }

    int bst_node_cnt, bst_leaf_cnt, bst_h;
    bst_stats(bst_root, &bst_node_cnt, &bst_leaf_cnt, &bst_h);

    int avl_node_cnt, avl_leaf_cnt;
    avl_stats(avl_root, &avl_node_cnt, &avl_leaf_cnt);
    int avl_h = avl_height(avl_root);

    printf("\n===== 2. 삽입(생성) 결과 =====\n");
    printf("  생성(시도)한 정수 개수          : %d\n", DRAW_N);
    printf("  실제로 저장된 서로 다른 값의 수 : %d\n", arr_len);
    printf("  중복으로 삽입되지 않은 값의 수  : %d\n", duplicates);
    if (mismatch_warn == 0 && bst_node_cnt == arr_len && avl_node_cnt == arr_len)
        printf("  (검증) 배열/BST/AVL 에 저장된 값의 개수가 모두 %d 로 일치합니다.\n", arr_len);
    else
        printf("  (경고) 세 자료구조에 저장된 개수가 불일치합니다! array=%d bst=%d avl=%d\n",
            arr_len, bst_node_cnt, avl_node_cnt);

    printf("\n  실제로 저장된 서로 다른 값 목록 (최초 발생 순서, %d개)\n", arr_len);
    for (int i = 0; i < arr_len; i++) {
        printf("%4d%s", arr[i], (i == arr_len - 1) ? "\n" : ", ");
        if ((i + 1) % 10 == 0) printf("\n");
    }

    printf("\nConstruction (생성 과정 비교 횟수)\n");
    printf("  Array comparisons : %6ld\n", array_cmp);
    printf("  BST   comparisons : %6ld\n", bst_cmp);
    printf("  AVL   comparisons : %6ld\n", avl_cmp);

    printf("\nStructure (최종 구조)\n");
    printf("  Array length : %3d\n", arr_len);
    printf("  BST height   : %3d   (단말 노드 수 %d / 전체 %d)\n", bst_h, bst_leaf_cnt, bst_node_cnt);
    printf("  AVL height   : %3d   (단말 노드 수 %d / 전체 %d)\n", avl_h, avl_leaf_cnt, avl_node_cnt);

    int query[QUERY_N];
    for (int i = 0; i < QUERY_N; i++) query[i] = rand_int();

    printf("\n===== 3. 탐색 대상 (%d개) =====\n", QUERY_N);
    for (int i = 0; i < QUERY_N; i++) {
        printf("%4d%s", query[i], (i == QUERY_N - 1) ? "\n" : ", ");
        if ((i + 1) % 10 == 0) printf("\n");
    }

    long seqc[QUERY_N], bstc[QUERY_N], avlc[QUERY_N];
    int  found[QUERY_N];
    long seq_total = 0, bst_total = 0, avl_total = 0;
    int  success_cnt = 0;

    for (int i = 0; i < QUERY_N; i++) {
        long sc = 0, bc = 0, ac = 0;
        int sf = array_search(arr, arr_len, query[i], &sc);
        int bf = bst_search(bst_root, query[i], &bc);
        int af = avl_search(avl_root, query[i], &ac);

        if (sf != bf || sf != af) {
            fprintf(stderr, "[경고] 탐색 키 %d 에서 결과가 불일치합니다! "
                "(array=%d, bst=%d, avl=%d)\n", query[i], sf, bf, af);
        }

        seqc[i] = sc; bstc[i] = bc; avlc[i] = ac; found[i] = sf;
        seq_total += sc; bst_total += bc; avl_total += ac;
        if (sf) success_cnt++;
    }

    printf("\n===== 4. 탐색 결과 =====\n");
    printf("%-13s %-13s %14s %12s %12s\n",
        "탐색 키", "결과", "순차 비교횟수", "BST 비교횟수", "AVL 비교횟수");
    printf("----------------------------------------------------------------\n");
    for (int i = 0; i < QUERY_N; i++) {
        printf("%-10d %-6s %14ld %12ld %12ld\n",
            query[i], found[i] ? "성공" : "실패", seqc[i], bstc[i], avlc[i]);
    }

    printf("\n----- 출력 형식 예시 (첫 번째 탐색 키) -----\n");
    printf("Search Key : %d\n\n", query[0]);
    printf("Sequential Search\n");
    printf("Result      : %s\n", found[0] ? "Found" : "Not Found");
    printf("Comparisons : %ld\n\n", seqc[0]);
    printf("BST Search\n");
    printf("Result      : %s\n", found[0] ? "Found" : "Not Found");
    printf("Comparisons : %ld\n\n", bstc[0]);
    printf("AVL Search\n");
    printf("Result      : %s\n", found[0] ? "Found" : "Not Found");
    printf("Comparisons : %ld\n", avlc[0]);

    char b1[32], b2[32], b3[32], b4[32], b5[32], b6[32];
    fmt_comma(array_cmp, b4, sizeof(b4));
    fmt_comma(bst_cmp, b5, sizeof(b5));
    fmt_comma(avl_cmp, b6, sizeof(b6));

    printf("\n========== 요약 (과제 출력 형식) ==========\n");
    printf("Stored values : %d\n\n", arr_len);

    printf("Construction\n");
    printf("Array comparisons : %6s\n", b4);
    printf("BST comparisons   : %6s\n", b5);
    printf("AVL comparisons   : %6s\n\n", b6);

    printf("Structure\n");
    printf("Array length : %d\n", arr_len);
    printf("BST height   : %d\n", bst_h);
    printf("AVL height   : %d\n\n", avl_h);

    printf("Searches : %d\n\n", QUERY_N);

    fmt_comma(seq_total, b1, sizeof(b1));
    printf("Sequential Search\n");
    printf("Total comparisons   : %s\n", b1);
    printf("Average comparisons : %.2f\n\n", (double)seq_total / QUERY_N);

    fmt_comma(bst_total, b2, sizeof(b2));
    printf("BST Search\n");
    printf("Total comparisons   : %s\n", b2);
    printf("Average comparisons : %.2f\n\n", (double)bst_total / QUERY_N);

    fmt_comma(avl_total, b3, sizeof(b3));
    printf("AVL Search\n");
    printf("Total comparisons   : %s\n", b3);
    printf("Average comparisons : %.2f\n", (double)avl_total / QUERY_N);

    printf("\n(참고) 탐색 성공 %d건 / 실패 %d건\n", success_cnt, QUERY_N - success_cnt);

    bst_free(bst_root);
    avl_free(avl_root);
    return 0;
}
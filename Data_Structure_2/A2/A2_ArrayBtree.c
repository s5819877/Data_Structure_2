#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN     1024
#define MAX_INDEX  (1 << 22)   
#define EMPTY      '\0'

static char buf[MAXLEN];
static int  blen;
static int  bpos;

static char* tree = NULL;
static long  arr_size = 0;
static long  max_index = 0;

static void error_exit(const char* msg)
{
    printf("\n[오류] %s\n", msg);
    printf("올바른 이진트리의 괄호 표기법이 아닙니다. 프로그램을 종료합니다5.\n");
    exit(1);
}

static void parse_tree(long idx)
{
    if (bpos >= blen) error_exit("노드가 와야 할 위치에서 입력이 끝났습니다.");

    char c = buf[bpos];
    if (c < 'A' || c > 'Z')
        error_exit("노드 이름은 영문 대문자 한 글자여야 합니다.");

    if (idx >= MAX_INDEX)
        error_exit("트리가 너무 깊어 배열로 표현할 수 없습니다. (깊이 22 초과)");

    if (idx >= arr_size) {
        long newsize = (arr_size == 0) ? 2 : arr_size;
        while (idx >= newsize) newsize *= 2;
        char* tmp = (char*)realloc(tree, (size_t)newsize * sizeof(char));
        if (!tmp) error_exit("메모리 할당에 실패했습니다.");
        memset(tmp + arr_size, EMPTY, (size_t)(newsize - arr_size) * sizeof(char));
        tree = tmp;
        arr_size = newsize;
    }

    tree[idx] = c;
    if (idx > max_index) max_index = idx;
    bpos++;

    if (bpos < blen && buf[bpos] == '(') {
        bpos++;
        if (bpos < blen && buf[bpos] != ',' && buf[bpos] != ')')
            parse_tree(idx * 2);
        if (bpos < blen && buf[bpos] == ',') {
            bpos++;
            if (bpos < blen && buf[bpos] != ')')
                parse_tree(idx * 2 + 1);
        }
        if (bpos >= blen || buf[bpos] != ')')
            error_exit("닫는 괄호 ')' 가 없습니다.");
        bpos++;
    }
}

static int exist(long idx)
{
    return (idx >= 1 && idx <= max_index && idx < arr_size && tree[idx] != EMPTY);
}

static void print_tree(long idx, int depth, const char* tag)
{
    if (!exist(idx)) return;

    if (depth == 0) {
        printf("%c\n", tree[idx]);
    }
    else {
        for (int k = 0; k < depth - 1; k++) printf("    ");
        printf("+---%c%s\n", tree[idx], tag);
    }
    print_tree(idx * 2, depth + 1, " (L)");
    print_tree(idx * 2 + 1, depth + 1, " (R)");
}

static void get_info(long idx, int depth,
    int* total, int* leaf, int* nonleaf,
    int* height, int* degree)
{
    if (!exist(idx)) return;

    (*total)++;
    if (depth + 1 > *height) *height = depth + 1;

    int d = 0;
    if (exist(idx * 2))     d++;
    if (exist(idx * 2 + 1)) d++;

    if (d == 0) (*leaf)++; else (*nonleaf)++;
    if (d > *degree) *degree = d;

    get_info(idx * 2, depth + 1, total, leaf, nonleaf, height, degree);
    get_info(idx * 2 + 1, depth + 1, total, leaf, nonleaf, height, degree);
}

static void print_info(void)
{
    int total = 0, leaf = 0, nonleaf = 0, height = 0, degree = 0;
    get_info(1, 0, &total, &leaf, &nonleaf, &height, &degree);

    printf("\n=== [배열 구현] 트리 정보 ===\n");
    printf("  1. 전체 노드의 수   : %d\n", total);
    printf("  2. 단말 노드의 수   : %d\n", leaf);
    printf("  3. 비단말 노드의 수 : %d\n", nonleaf);
    printf("  4. 트리의 높이      : %d\n", height);
    printf("  5. 트리의 차수      : %d\n", degree);
}

static int is_complete(int total)
{
    for (int i = 1; i <= total; i++)
        if (!exist(i)) return 0;
    return 1;
}

static int is_full(int total, int height)
{
    long full_nodes = (1L << height) - 1;
    return (is_complete(total) && total == full_nodes);
}

static int skew_kind(void)
{
    int has_left = 0, has_right = 0, n = 0;

    for (long i = 1; i <= max_index && i < arr_size; i++) {
        if (tree[i] == EMPTY) continue;
        n++;
        int l = exist(i * 2);
        int r = exist(i * 2 + 1);
        if (l && r) return 0;
        if (l) has_left = 1;
        if (r) has_right = 1;
    }
    if (n == 1)                 return 3;
    if (has_left && has_right)  return 4;
    if (has_left)               return 1;
    if (has_right)              return 2;
    return 3;
}

static void print_shape(void)
{
    int total = 0, leaf = 0, nonleaf = 0, height = 0, degree = 0;
    get_info(1, 0, &total, &leaf, &nonleaf, &height, &degree);

    int comp = is_complete(total);
    int full = is_full(total, height);
    int sk = skew_kind();

    printf("\n=== [배열 구현] 이진트리 형태 판별 ===\n");
    printf("  완전 이진트리 여부 : %s\n", comp ? "예" : "아니오");
    printf("  포화 이진트리 여부 : %s\n", full ? "예" : "아니오");
    printf("  편향 이진트리 여부 : ");
    if (sk == 1) printf("예 (왼쪽 편향)\n");
    else if (sk == 2) printf("예 (오른쪽 편향)\n");
    else if (sk == 3) printf("예 (노드가 1개)\n");
    else if (sk == 4) printf("예 (모든 노드의 자식이 1개 이하, 방향 혼합)\n");
    else              printf("아니오\n");
}

static void print_memory(void)
{
    int total = 0, leaf = 0, nonleaf = 0, height = 0, degree = 0;
    get_info(1, 0, &total, &leaf, &nonleaf, &height, &degree);

    size_t bytes = (size_t)(max_index + 1) * sizeof(char);
    long   empty_slots = (max_index + 1) - total - 1;

    printf("\n=== [배열 구현] 메모리 사용량 ===\n");
    printf("  실제 노드 수        : %d\n", total);
    printf("  트리 높이           : %d\n", height);
    printf("  사용된 최대 인덱스  : %ld\n", max_index);
    printf("  필요한 배열 크기    : %ld 칸 (인덱스 0 ~ %ld)\n", max_index + 1, max_index);
    printf("  원소 1개 크기       : %zu byte (char)\n", sizeof(char));
    printf("  --------------------------------------------\n");
    printf("  총 메모리 사용량    : %zu byte\n", bytes);
    printf("  비어 있는 칸        : %ld 칸 (%.1f%%)\n",
        empty_slots, 100.0 * (double)(max_index - total) / (double)(max_index + 1));
    printf("  --------------------------------------------\n");
    printf("  ※ 배열은 노드 수가 아니라 '높이'가 메모리를 결정한다.\n");
    printf("     높이 h 인 트리는 최대 2^h - 1 칸이 필요하므로,\n");
    printf("     편향 트리일수록 빈 칸이 기하급수적으로 늘어난다.\n");
}

static void query_node(char target)
{
    long idx = -1;
    long steps = 0;

    for (long i = 1; i <= max_index && i < arr_size; i++) {
        steps++;
        if (tree[i] == target) { idx = i; break; }
    }
    if (idx < 0) {
        printf("\n노드 '%c' 를 트리에서 찾을 수 없습니다.\n", target);
        return;
    }

    printf("\n=== [배열 구현] 노드 '%c' 의 관계 노드 ===\n", target);
    printf("  노드 위치 : 인덱스 %ld\n", idx);
    printf("  ----------------------------------------\n");

    if (idx == 1)
        printf("  부모 노드      : 없음 (루트 노드)\n");
    else
        printf("  부모 노드      : %c   (인덱스 %ld = %ld/2, 계산 1회)\n",
            tree[idx / 2], idx / 2, idx);

    long l = idx * 2, r = idx * 2 + 1;
    if (exist(l)) printf("  왼쪽 자식      : %c   (인덱스 %ld = %ld*2)\n", tree[l], l, idx);
    else          printf("  왼쪽 자식      : 없음\n");
    if (exist(r)) printf("  오른쪽 자식    : %c   (인덱스 %ld = %ld*2+1)\n", tree[r], r, idx);
    else          printf("  오른쪽 자식    : 없음\n");

    if (idx == 1) {
        printf("  형제 노드      : 없음 (루트 노드)\n");
    }
    else {
        long sib = (idx % 2 == 0) ? idx + 1 : idx - 1;
        if (exist(sib))
            printf("  형제 노드      : %c   (인덱스 %ld, 계산 1회)\n", tree[sib], sib);
        else
            printf("  형제 노드      : 없음\n");
    }

    printf("  ----------------------------------------\n");
    printf("  탐색 비용 : 노드 위치 검색 %ld회 비교\n", steps);
    printf("              부모 / 자식 / 형제 접근은 모두 O(1) 산술 계산\n");
    printf("              (부모 = i/2, 자식 = 2i · 2i+1, 형제 = i±1)\n");
}

static void read_tree(void)
{
    char raw[MAXLEN];

    printf("\n이진트리의 괄호 표기법을 입력하세요 (예: A(B(D,E),C(,F))): ");
    if (!fgets(raw, sizeof(raw), stdin)) error_exit("입력을 읽을 수 없습니다.");

    int j = 0;
    for (int i = 0; raw[i]; i++)
        if (!isspace((unsigned char)raw[i])) buf[j++] = raw[i];
    buf[j] = '\0';
    blen = j;
    if (blen == 0) error_exit("입력이 비어 있습니다.");

    free(tree);
    tree = NULL;
    arr_size = 0;
    max_index = 0;

    bpos = 0;
    parse_tree(1);
    if (bpos != blen) error_exit("트리 표현이 끝난 뒤에 불필요한 문자가 있습니다.");

    printf("\n입력된 이진트리 : %s\n", buf);
    printf("배열을 이용한 이진트리가 생성되었습니다. (최대 인덱스 %ld)\n", max_index);
}

int main(void)
{
    printf("========================================================\n");
    printf("      과제-02 : 배열을 이용한 이진트리 구현\n");
    printf("========================================================\n");

    read_tree();

    int menu;
    while (1) {
        printf("\n---------------- 메 뉴 ----------------\n");
        printf("  1) 이진트리 출력\n");
        printf("  2) 트리 정보 출력\n");
        printf("  3) 이진트리 형태 판별 (완전/포화/편향)\n");
        printf("  4) 메모리 사용량 측정\n");
        printf("  5) 특정 노드의 자식/부모/형제 탐색\n");
        printf("  6) 새로운 트리 입력\n");
        printf("  0) 종료\n");
        printf("---------------------------------------\n");
        printf("선택 > ");

        if (scanf("%d", &menu) != 1) break;
        while (getchar() != '\n');

        switch (menu) {
        case 1:
            printf("\n=== [배열 구현] 이진트리 출력 ===\n");
            print_tree(1, 0, "");
            break;
        case 2:
            print_info();
            break;
        case 3:
            print_shape();
            break;
        case 4:
            print_memory();
            break;
        case 5: {
            char t;
            printf("\n찾을 노드를 입력하세요 (영문 대문자 한 글자) : ");
            if (scanf(" %c", &t) != 1) break;
            while (getchar() != '\n');
            query_node((char)toupper((unsigned char)t));
            break;
        }
        case 6:
            read_tree();
            break;
        case 0:
            free(tree);
            printf("프로그램을 종료합니다.\n");
            return 0;
        default:
            printf("잘못된 선택입니다.\n");
        }
    }

    free(tree);
    return 0;
}
#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define maxlen 1024

static char buf[maxlen];
static int  blen;
static int  bpos;

typedef struct treenode {
    char             data;
    struct treenode* left;
    struct treenode* right;
} treenode;

static treenode* root = NULL;
static int  node_count = 0;

static void error_exit(const char* msg)
{
    printf("\n[오류] %s\n", msg);
    printf("올바른 이진트리의 괄호 표기법이 아닙니다. 프로그램을 종료합니다.\n");
    exit(1);
}

static treenode* parse_tree(void)
{
    if (bpos >= blen) error_exit("노드가 와야 할 위치에서 입력이 끝났습니다.");

    char c = buf[bpos];
    if (c < 'A' || c > 'Z')
        error_exit("노드 이름은 영문 대문자 한 글자여야 합니다.");

    treenode* p = (treenode*)malloc(sizeof(treenode));
    if (!p) error_exit("메모리 할당에 실패했습니다.");
    p->data = c;
    p->left = NULL;
    p->right = NULL;
    node_count++;
    bpos++;

    if (bpos < blen && buf[bpos] == '(') {
        bpos++;
        if (bpos < blen && buf[bpos] != ',' && buf[bpos] != ')')
            p->left = parse_tree();
        if (bpos < blen && buf[bpos] == ',') {
            bpos++;
            if (bpos < blen && buf[bpos] != ')')
                p->right = parse_tree();
        }
        if (bpos >= blen || buf[bpos] != ')')
            error_exit("닫는 괄호 ')' 가 없습니다.");
        bpos++;
    }
    return p;
}

static void print_tree(treenode* p, int depth, const char* tag)
{
    if (!p) return;

    if (depth == 0) {
        printf("%c\n", p->data);
    }
    else {
        for (int k = 0; k < depth - 1; k++) printf("    ");
        printf("+---%c%s\n", p->data, tag);
    }
    print_tree(p->left, depth + 1, " (L)");
    print_tree(p->right, depth + 1, " (R)");
}

static void get_info(treenode* p, int depth,
    int* total, int* leaf, int* nonleaf,
    int* height, int* degree)
{
    if (!p) return;

    (*total)++;
    if (depth + 1 > *height) *height = depth + 1;

    int d = 0;
    if (p->left)  d++;
    if (p->right) d++;

    if (d == 0) (*leaf)++; else (*nonleaf)++;
    if (d > *degree) *degree = d;

    get_info(p->left, depth + 1, total, leaf, nonleaf, height, degree);
    get_info(p->right, depth + 1, total, leaf, nonleaf, height, degree);
}

static void print_info(void)
{
    int total = 0, leaf = 0, nonleaf = 0, height = 0, degree = 0;
    get_info(root, 0, &total, &leaf, &nonleaf, &height, &degree);

    printf("\n=== [연결 구현] 트리 정보 ===\n");
    printf("  1. 전체 노드의 수   : %d\n", total);
    printf("  2. 단말 노드의 수   : %d\n", leaf);
    printf("  3. 비단말 노드의 수 : %d\n", nonleaf);
    printf("  4. 트리의 높이      : %d\n", height);
    printf("  5. 트리의 차수      : %d\n", degree);
}

static int is_complete(treenode* r)
{
    if (!r) return 1;

    treenode* queue[maxlen];
    int front = 0, rear = 0;
    int seen_null = 0;

    queue[rear++] = r;
    while (front < rear) {
        treenode* cur = queue[front++];
        if (cur == NULL) {
            seen_null = 1;
        }
        else {
            if (seen_null) return 0;
            queue[rear++] = cur->left;
            queue[rear++] = cur->right;
        }
    }
    return 1;
}

static int is_full(treenode* r, int total, int height)
{
    long full_nodes = (1l << height) - 1;
    return (is_complete(r) && total == full_nodes);
}

static void skew_check(treenode* p, int* two_child, int* has_l, int* has_r)
{
    if (!p) return;
    if (p->left && p->right) { *two_child = 1; return; }
    if (p->left)  *has_l = 1;
    if (p->right) *has_r = 1;
    skew_check(p->left, two_child, has_l, has_r);
    skew_check(p->right, two_child, has_l, has_r);
}

static void print_shape(void)
{
    int total = 0, leaf = 0, nonleaf = 0, height = 0, degree = 0;
    get_info(root, 0, &total, &leaf, &nonleaf, &height, &degree);

    int comp = is_complete(root);
    int full = is_full(root, total, height);
    int two = 0, hl = 0, hr = 0;
    skew_check(root, &two, &hl, &hr);

    printf("\n=== [연결 구현] 이진트리 형태 판별 ===\n");
    printf("  완전 이진트리 여부 : %s\n", comp ? "예" : "아니오");
    printf("  포화 이진트리 여부 : %s\n", full ? "예" : "아니오");
    printf("  편향 이진트리 여부 : ");
    if (two)          printf("아니오\n");
    else if (total == 1)   printf("예 (노드가 1개)\n");
    else if (hl && hr)     printf("예 (모든 노드의 자식이 1개 이하, 방향 혼합)\n");
    else if (hl)           printf("예 (왼쪽 편향)\n");
    else                   printf("예 (오른쪽 편향)\n");
}

static void print_memory(void)
{
    int total = 0, leaf = 0, nonleaf = 0, height = 0, degree = 0;
    get_info(root, 0, &total, &leaf, &nonleaf, &height, &degree);

    size_t node_bytes = sizeof(treenode);
    size_t bytes = (size_t)node_count * node_bytes;
    size_t data_bytes = (size_t)node_count * sizeof(char);

    printf("\n=== [연결 구현] 메모리 사용량 ===\n");
    printf("  실제 노드 수        : %d\n", total);
    printf("  트리 높이           : %d\n", height);
    printf("  노드 1개 크기       : %zu byte\n", node_bytes);
    printf("     - data (char)    : %zu byte\n", sizeof(char));
    printf("     - left  포인터   : %zu byte\n", sizeof(treenode*));
    printf("     - right 포인터   : %zu byte\n", sizeof(treenode*));
    printf("     - 정렬 패딩      : %zu byte\n",
        node_bytes - sizeof(char) - 2 * sizeof(treenode*));
    printf("  --------------------------------------------\n");
    printf("  총 메모리 사용량    : %zu byte  (%d개 x %zu byte)\n",
        bytes, node_count, node_bytes);
    printf("  실제 데이터         : %zu byte (%.1f%%)\n",
        data_bytes, 100.0 * (double)data_bytes / (double)bytes);
    printf("  링크 · 패딩 오버헤드: %zu byte (%.1f%%)\n",
        bytes - data_bytes, 100.0 * (double)(bytes - data_bytes) / (double)bytes);
    printf("  --------------------------------------------\n");
    printf("  ※ 연결 구현의 메모리는 트리의 형태와 무관하게\n");
    printf("     오직 노드 수에만 비례한다. (o(n))\n");
    printf("     대신 노드 1개당 포인터 오버헤드가 크다.\n");
}

static long steps;

static treenode* find_node(treenode* p, char target)
{
    if (!p) return NULL;
    steps++;
    if (p->data == target) return p;

    treenode* f = find_node(p->left, target);
    if (f) return f;
    return find_node(p->right, target);
}

static treenode* find_parent(treenode* p, treenode* child)
{
    if (!p || p == child) return NULL;
    steps++;
    if (p->left == child || p->right == child) return p;

    treenode* f = find_parent(p->left, child);
    if (f) return f;
    return find_parent(p->right, child);
}

static void query_node(char target)
{
    steps = 0;
    treenode* node = find_node(root, target);
    long find_steps = steps;

    if (!node) {
        printf("\n노드 '%c' 를 트리에서 찾을 수 없습니다.\n", target);
        return;
    }

    printf("\n=== [연결 구현] 노드 '%c' 의 관계 노드 ===\n", target);
    printf("  노드 주소 : %p\n", (void*)node);
    printf("  ----------------------------------------\n");

    steps = 0;
    treenode* par = find_parent(root, node);
    long par_steps = steps;

    if (!par) printf("  부모 노드      : 없음 (루트 노드)\n");
    else      printf("  부모 노드      : %c   (루트부터 %ld회 재방문하여 탐색)\n",
        par->data, par_steps);

    if (node->left)  printf("  왼쪽 자식      : %c   (p->left,  포인터 1회 역참조)\n",
        node->left->data);
    else             printf("  왼쪽 자식      : 없음 (p->left == null)\n");
    if (node->right) printf("  오른쪽 자식    : %c   (p->right, 포인터 1회 역참조)\n",
        node->right->data);
    else             printf("  오른쪽 자식    : 없음 (p->right == null)\n");

    if (!par) {
        printf("  형제 노드      : 없음 (루트 노드)\n");
    }
    else {
        treenode* sib = (par->left == node) ? par->right : par->left;
        if (sib) printf("  형제 노드      : %c   (부모를 찾은 뒤 1회 역참조)\n", sib->data);
        else     printf("  형제 노드      : 없음\n");
    }

    printf("  ----------------------------------------\n");
    printf("  탐색 비용 : 노드 검색 %ld회 방문 + 부모 탐색 %ld회 재방문\n",
        find_steps, par_steps);
    printf("              자식 접근은 o(1) 이지만,\n");
    printf("              부모 포인터가 없어 부모 · 형제 접근은 o(n) 이다.\n");
}

static void free_tree(treenode* p)
{
    if (!p) return;
    free_tree(p->left);
    free_tree(p->right);
    free(p);
}

static void read_tree(void)
{
    char raw[maxlen];

    printf("\n이진트리의 괄호 표기법을 입력하세요 (예: A(B(D,E),C(,F))): ");
    if (!fgets(raw, sizeof(raw), stdin)) error_exit("입력을 읽을 수 없습니다.");

    int j = 0;
    for (int i = 0; raw[i]; i++)
        if (!isspace((unsigned char)raw[i])) buf[j++] = raw[i];
    buf[j] = '\0';
    blen = j;
    if (blen == 0) error_exit("입력이 비어 있습니다.");

    free_tree(root);
    root = NULL;
    node_count = 0;

    bpos = 0;
    root = parse_tree();
    if (bpos != blen) error_exit("트리 표현이 끝난 뒤에 불필요한 문자가 있습니다.");

    printf("\n입력된 이진트리 : %s\n", buf);
    printf("포인터를 이용한 연결 이진트리가 생성되었습니다. (노드 %d개)\n", node_count);
}

int main(void)
{
    printf("========================================================\n");
    printf("   과제-02 : 포인터를 이용한 연결 이진트리 구현\n");
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
            printf("\n=== [연결 구현] 이진트리 출력 ===\n");
            print_tree(root, 0, "");
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
            free_tree(root);
            printf("프로그램을 종료합니다.\n");
            return 0;
        default:
            printf("잘못된 선택입니다.\n");
        }
    }

    free_tree(root);
    return 0;
}
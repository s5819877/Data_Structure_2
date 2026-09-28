
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "A4_TreeOrder.h"


typedef struct {
    const TreeNode* node;
    int             depth;
    char            tag;
} Item;

typedef struct {
    Item* a;
    int   top;
    int   cap;
} Stack;

static void st_init(Stack* s)
{
    s->cap = 64;
    s->top = 0;
    s->a = (Item*)malloc((size_t)s->cap * sizeof(Item));
    if (!s->a) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
}

static void st_free(Stack* s) { free(s->a); s->a = NULL; s->top = s->cap = 0; }
static int  st_empty(const Stack* s) { return s->top == 0; }

static void st_push(Stack* s, const TreeNode* node, int depth, char tag)
{
    if (s->top == s->cap) {
        s->cap *= 2;
        Item* tmp = (Item*)realloc(s->a, (size_t)s->cap * sizeof(Item));
        if (!tmp) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
        s->a = tmp;
    }
    s->a[s->top].node = node;
    s->a[s->top].depth = depth;
    s->a[s->top].tag = tag;
    s->top++;
}

static Item st_pop(Stack* s) { return s->a[--s->top]; }
static const TreeNode* st_peek(const Stack* s) { return s->a[s->top - 1].node; }

typedef struct {
    TreeNode* node;
    int       slot;
    int       nchild;
} PFrame;

static TreeNode* new_node(char c)
{
    TreeNode* n = (TreeNode*)malloc(sizeof(TreeNode));
    if (!n) return NULL;
    n->data = c;
    n->left = n->right = NULL;
    return n;
}

#define FAIL(...) do { snprintf(err, errlen, __VA_ARGS__); goto fail; } while (0)

TreeNode* build_tree(const char* raw, char* err, size_t errlen)
{
    enum { EXPECT_ROOT, EXPECT_CHILD, AFTER_CHILD, DONE } state = EXPECT_ROOT;

    size_t rawlen = strlen(raw);
    char* buf = (char*)malloc(rawlen + 1);
    PFrame* fr = (PFrame*)malloc((rawlen + 1) * sizeof(PFrame));
    TreeNode* root = NULL;
    int top = 0;
    size_t pos = 0;

    if (!buf || !fr) { free(buf); free(fr); snprintf(err, errlen, "메모리 할당 실패"); return NULL; }

    /* 공백/개행 제거 */
    size_t len = 0;
    for (size_t i = 0; i < rawlen; i++)
        if (!isspace((unsigned char)raw[i])) buf[len++] = raw[i];
    buf[len] = '\0';

    if (len == 0) FAIL("입력이 비어 있습니다.");

    while (state != DONE) {
        char c = buf[pos];

        if (state == EXPECT_ROOT || state == EXPECT_CHILD) {
            if (c >= 'A' && c <= 'Z') { 
                TreeNode* n = new_node(c);
                if (!n) FAIL("메모리 할당 실패");

                if (state == EXPECT_ROOT) {
                    root = n;
                }
                else { 
                    PFrame* f = &fr[top - 1];
                    if (f->slot == 0) f->node->left = n;
                    else              f->node->right = n;
                    f->nchild++;
                }
                pos++;

                if (buf[pos] == '(') {
                    fr[top].node = n; fr[top].slot = 0; fr[top].nchild = 0;
                    top++;
                    pos++;
                    state = EXPECT_CHILD;
                }
                else {
                    state = (top == 0) ? DONE : AFTER_CHILD;
                }
            }
            else if (state == EXPECT_CHILD && c == ',') {
                if (fr[top - 1].slot == 1)
                    FAIL("%zu번째 문자: 한 노드에 자식이 3개 이상 올 수 없습니다.", pos + 1);
                fr[top - 1].slot = 1;
                pos++;
            }
            else if (state == EXPECT_CHILD && c == ')') {
                if (fr[top - 1].nchild == 0)
                    FAIL("%zu번째 문자: 빈 괄호 '()' 는 사용할 수 없습니다.", pos + 1);
                top--;
                pos++;
                state = (top == 0) ? DONE : AFTER_CHILD;
            }
            else if (c == '\0') {
                FAIL("입력이 갑자기 끝났습니다. (노드 또는 닫는 괄호가 필요합니다)");
            }
            else {
                FAIL("%zu번째 문자 '%c': 노드 이름은 영문 대문자 한 글자여야 하며, "
                    "이 위치에는 사용할 수 없는 문자입니다.", pos + 1, c);
            }
        }
        else { 
            if (c == ',') {
                if (fr[top - 1].slot == 1)
                    FAIL("%zu번째 문자: 한 노드에 자식이 3개 이상 올 수 없습니다.", pos + 1);
                fr[top - 1].slot = 1;
                pos++;
                state = EXPECT_CHILD;
            }
            else if (c == ')') {
                top--;
                pos++;
                state = (top == 0) ? DONE : AFTER_CHILD;
            }
            else if (c == '\0') {
                FAIL("닫는 괄호 ')' 가 부족합니다.");
            }
            else {
                FAIL("%zu번째 문자 '%c': ',' 또는 ')' 가 와야 합니다.", pos + 1, c);
            }
        }
    }

    if (buf[pos] != '\0')
        FAIL("%zu번째 문자 '%c': 트리 표현이 이미 끝났는데 뒤에 문자가 남아 있습니다.",
            pos + 1, buf[pos]);

    free(buf);
    free(fr);
    return root;

fail:
    free(buf);
    free(fr);
    free_tree(root);
    return NULL;
}

void print_tree(const TreeNode* root)
{
    if (!root) { printf("(트리가 비어 있습니다)\n"); return; }

    Stack s;
    st_init(&s);
    st_push(&s, root, 0, '\0');

    while (!st_empty(&s)) {
        Item it = st_pop(&s);

        if (it.depth == 0) {
            printf("%c\n", it.node->data);
        }
        else {
            for (int k = 0; k < it.depth - 1; k++) printf("    ");
            printf("+---%c (%c)\n", it.node->data, it.tag);
        }
        if (it.node->right) st_push(&s, it.node->right, it.depth + 1, 'R');
        if (it.node->left)  st_push(&s, it.node->left, it.depth + 1, 'L');
    }
    st_free(&s);
}

static void visit(char data, int* first)
{
    if (!*first) printf(" ");
    printf("%c", data);
    *first = 0;
}

void preorder(const TreeNode* root)
{
    Stack s;
    int first = 1;

    printf("Preorder  : ");
    if (root) {
        st_init(&s);
        st_push(&s, root, 0, 0);
        while (!st_empty(&s)) {
            const TreeNode* cur = st_pop(&s).node;
            visit(cur->data, &first); 
            if (cur->right) st_push(&s, cur->right, 0, 0);
            if (cur->left)  st_push(&s, cur->left, 0, 0);
        }
        st_free(&s);
    }
    printf("\n");
}

void inorder(const TreeNode* root)
{
    Stack s;
    int first = 1;
    const TreeNode* cur = root;

    printf("Inorder   : ");
    st_init(&s);
    while (cur || !st_empty(&s)) {
        while (cur) { 
            st_push(&s, cur, 0, 0);
            cur = cur->left;
        }
        cur = st_pop(&s).node;
        visit(cur->data, &first);
        cur = cur->right;
    }
    st_free(&s);
    printf("\n");
}

void postorder(const TreeNode* root)
{
    Stack s;
    int first = 1;
    const TreeNode* cur = root;
    const TreeNode* last = NULL; 

    printf("Postorder : ");
    st_init(&s);
    while (cur || !st_empty(&s)) {
        while (cur) {
            st_push(&s, cur, 0, 0);
            cur = cur->left;
        }
        const TreeNode* top = st_peek(&s);
        if (top->right && top->right != last) {
            cur = top->right;
        }
        else {
            visit(top->data, &first);
            last = top;
            st_pop(&s);
        }
    }
    st_free(&s);
    printf("\n");
}

long count_nodes(const TreeNode* root)
{
    long n = 0;
    if (!root) return 0;

    Stack s;
    st_init(&s);
    st_push(&s, root, 0, 0);
    while (!st_empty(&s)) {
        const TreeNode* cur = st_pop(&s).node;
        n++;
        if (cur->right) st_push(&s, cur->right, 0, 0);
        if (cur->left)  st_push(&s, cur->left, 0, 0);
    }
    st_free(&s);
    return n;
}

void free_tree(TreeNode* root)
{
    if (!root) return;

    Stack s;
    st_init(&s);
    st_push(&s, root, 0, 0);
    while (!st_empty(&s)) {
        TreeNode* cur = (TreeNode*)st_pop(&s).node;
        if (cur->right) st_push(&s, cur->right, 0, 0);
        if (cur->left)  st_push(&s, cur->left, 0, 0);
        free(cur);
    }
    st_free(&s);
}
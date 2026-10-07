#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "A6_BST.h"

static BSTNode* new_node(int value)
{
    BSTNode* n = (BSTNode*)malloc(sizeof(BSTNode));
    if (!n) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    n->data = value;
    n->left = n->right = NULL;
    return n;
}

BSTNode* bst_insert(BSTNode* root, int value, long* cmp_count, int* inserted)
{
    if (!root) { 
        *inserted = 1;
        return new_node(value);
    }

    BSTNode* cur = root;
    while (1) {
        (*cmp_count)++;
        if (value == cur->data) {
            *inserted = 0;
            return root;
        }
        else if (value < cur->data) {
            if (cur->left) { cur = cur->left; }
            else { cur->left = new_node(value); *inserted = 1; return root; }
        }
        else {
            if (cur->right) { cur = cur->right; }
            else { cur->right = new_node(value); *inserted = 1; return root; }
        }
    }
}

int bst_search(const BSTNode* root, int target, long* cmp_count)
{
    const BSTNode* cur = root;
    while (cur) {
        (*cmp_count)++;
        if (target == cur->data) return 1;
        cur = (target < cur->data) ? cur->left : cur->right;
    }
    return 0;
}

void bst_stats(const BSTNode* root, int* node_count, int* leaf_count, int* height)
{
    *node_count = 0; *leaf_count = 0; *height = 0;
    if (!root) return;

    typedef struct { const BSTNode* n; int d; } Pair;
    int cap = 4, top = 0;
    Pair* stack = (Pair*)malloc(sizeof(Pair) * cap);
    stack[top].n = root; stack[top].d = 1; top++;

    while (top > 0) {
        Pair p = stack[--top];
        (*node_count)++;
        if (p.d > *height) *height = p.d;
        if (!p.n->left && !p.n->right) (*leaf_count)++;

        if (p.n->right) {
            if (top == cap) { cap *= 2; stack = (Pair*)realloc(stack, sizeof(Pair) * cap); }
            stack[top].n = p.n->right; stack[top].d = p.d + 1; top++;
        }
        if (p.n->left) {
            if (top == cap) { cap *= 2; stack = (Pair*)realloc(stack, sizeof(Pair) * cap); }
            stack[top].n = p.n->left; stack[top].d = p.d + 1; top++;
        }
    }
    free(stack);
}

void bst_free(BSTNode* root)
{
    if (!root) return;
    int cap = 4, top = 0;
    BSTNode** stack = (BSTNode**)malloc(sizeof(BSTNode*) * cap);
    stack[top++] = root;

    while (top > 0) {
        BSTNode* cur = stack[--top];
        if (cur->right) {
            if (top == cap) { cap *= 2; stack = (BSTNode**)realloc(stack, sizeof(BSTNode*) * cap); }
            stack[top++] = cur->right;
        }
        if (cur->left) {
            if (top == cap) { cap *= 2; stack = (BSTNode**)realloc(stack, sizeof(BSTNode*) * cap); }
            stack[top++] = cur->left;
        }
        free(cur);
    }
    free(stack);
}
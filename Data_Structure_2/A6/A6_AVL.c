#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "A6_AVL.h"

static int h_of(const AVLNode* n) { return n ? n->height : 0; }
static int max2(int a, int b) { return a > b ? a : b; }

static AVLNode* new_node(int value)
{
    AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
    if (!n) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    n->data = value;
    n->height = 1;
    n->left = n->right = NULL;
    return n;
}

static void update_height(AVLNode* n) { n->height = 1 + max2(h_of(n->left), h_of(n->right)); }
static int  balance_factor(const AVLNode* n) { return h_of(n->left) - h_of(n->right); }

static AVLNode* rotate_right(AVLNode* y)
{
    AVLNode* x = y->left;
    AVLNode* t2 = x->right;

    x->right = y;
    y->left = t2;

    update_height(y);
    update_height(x);
    return x;
}

static AVLNode* rotate_left(AVLNode* x)
{
    AVLNode* y = x->right;
    AVLNode* t2 = y->left;

    y->left = x;
    x->right = t2;

    update_height(x);
    update_height(y);
    return y;
}

AVLNode* avl_insert(AVLNode* node, int value, long* cmp_count, int* inserted)
{
    if (!node) {
        *inserted = 1;
        return new_node(value);
    }

    (*cmp_count)++; 
    if (value == node->data) {
        *inserted = 0;
        return node;
    }
    else if (value < node->data) {
        node->left = avl_insert(node->left, value, cmp_count, inserted);
    }
    else {
        node->right = avl_insert(node->right, value, cmp_count, inserted);
    }

    if (!*inserted) return node;

    update_height(node);
    int bf = balance_factor(node);

    if (bf > 1 && value < node->left->data) { 
        return rotate_right(node);
    }
    if (bf < -1 && value > node->right->data) {
        return rotate_left(node);
    }
    if (bf > 1 && value > node->left->data) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (bf < -1 && value < node->right->data) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

int avl_search(const AVLNode* root, int target, long* cmp_count)
{
    const AVLNode* cur = root;
    while (cur) {
        (*cmp_count)++;
        if (target == cur->data) return 1;
        cur = (target < cur->data) ? cur->left : cur->right;
    }
    return 0;
}

int avl_height(const AVLNode* root) { return h_of(root); }

void avl_stats(const AVLNode* root, int* node_count, int* leaf_count)
{
    *node_count = 0; *leaf_count = 0;
    if (!root) return;

    int cap = 4, top = 0;
    const AVLNode** stack = (const AVLNode**)malloc(sizeof(AVLNode*) * cap);
    stack[top++] = root;

    while (top > 0) {
        const AVLNode* cur = stack[--top];
        (*node_count)++;
        if (!cur->left && !cur->right) (*leaf_count)++;

        if (cur->right) {
            if (top == cap) { cap *= 2; stack = (const AVLNode**)realloc(stack, sizeof(AVLNode*) * cap); }
            stack[top++] = cur->right;
        }
        if (cur->left) {
            if (top == cap) { cap *= 2; stack = (const AVLNode**)realloc(stack, sizeof(AVLNode*) * cap); }
            stack[top++] = cur->left;
        }
    }
    free(stack);
}

void avl_free(AVLNode* root)
{
    if (!root) return;
    int cap = 4, top = 0;
    AVLNode** stack = (AVLNode**)malloc(sizeof(AVLNode*) * cap);
    stack[top++] = root;

    while (top > 0) {
        AVLNode* cur = stack[--top];
        if (cur->right) {
            if (top == cap) { cap *= 2; stack = (AVLNode**)realloc(stack, sizeof(AVLNode*) * cap); }
            stack[top++] = cur->right;
        }
        if (cur->left) {
            if (top == cap) { cap *= 2; stack = (AVLNode**)realloc(stack, sizeof(AVLNode*) * cap); }
            stack[top++] = cur->left;
        }
        free(cur);
    }
    free(stack);
}
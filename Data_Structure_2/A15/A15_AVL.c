#include <stdio.h>
#include <stdlib.h>
#include "A15_AVL.h"

static AvlNode* new_node(int v, Counters* c)
{
    AvlNode* t = (AvlNode*)malloc(sizeof(AvlNode));
    if (!t) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    t->key = v; c->mov++;                            
    t->count = 1; t->height = 1; t->left = t->right = NULL;
    c->bytes_now += sizeof(AvlNode);
    if (c->bytes_now > c->bytes_peak) c->bytes_peak = c->bytes_now;
    return t;
}
static void free_node(AvlNode* t, Counters* c) { free(t); c->bytes_now -= sizeof(AvlNode); }

static int h(const AvlNode* t) { return t ? t->height : 0; }
static void update(AvlNode* t) { int l = h(t->left), r = h(t->right); t->height = 1 + (l > r ? l : r); }
static int balance(const AvlNode* t) { return h(t->left) - h(t->right); }

static AvlNode* rot_right(AvlNode* y, Counters* c)
{
    AvlNode* x = y->left;
    y->left = x->right; x->right = y;
    update(y); update(x); c->rot++;
    return x;
}
static AvlNode* rot_left(AvlNode* x, Counters* c)
{
    AvlNode* y = x->right;
    x->right = y->left; y->left = x;
    update(x); update(y); c->rot++;
    return y;
}
static AvlNode* rebalance(AvlNode* t, Counters* c)
{
    update(t);
    int b = balance(t);
    if (b > 1) {                                     
        if (balance(t->left) < 0) t->left = rot_left(t->left, c);    
        return rot_right(t, c);
    }
    if (b < -1) {                                    
        if (balance(t->right) > 0) t->right = rot_right(t->right, c); 
        return rot_left(t, c);
    }
    return t;
}

static AvlNode* ins(AvlNode* t, int v, int depth, Counters* c)
{
    if (!t) return new_node(v, c);
    if (depth > c->max_depth) c->max_depth = depth;
    c->cmp++;                                        
    int d = (v > t->key) - (v < t->key);
    if (d < 0)      t->left = ins(t->left, v, depth + 1, c);
    else if (d > 0) t->right = ins(t->right, v, depth + 1, c);
    else { t->count++; return t; }                   
    return rebalance(t, c);
}

AvlNode* avl_insert(AvlNode* root, int v, Counters* c) { return ins(root, v, 1, c); }

static AvlNode* del_min(AvlNode* t, int depth, Counters* c)
{
    if (depth > c->max_depth) c->max_depth = depth;
    if (!t->left) { AvlNode* r = t->right; free_node(t, c); return r; }
    t->left = del_min(t->left, depth + 1, c);
    return rebalance(t, c);
}
static const AvlNode* min_node(const AvlNode* t) { while (t->left) t = t->left; return t; }

static AvlNode* del(AvlNode* t, int v, int depth, int* removed, Counters* c)
{
    if (!t) return NULL;                             
    if (depth > c->max_depth) c->max_depth = depth;
    c->cmp++;
    int d = (v > t->key) - (v < t->key);
    if (d < 0)      t->left = del(t->left, v, depth + 1, removed, c);
    else if (d > 0) t->right = del(t->right, v, depth + 1, removed, c);
    else {
        *removed = 1;
        if (t->count > 1) { t->count--; return t; }  
        if (!t->left || !t->right) {                 
            AvlNode* child = t->left ? t->left : t->right;
            free_node(t, c);
            return child;
        }
        const AvlNode* s = min_node(t->right);       
        t->key = s->key; t->count = s->count; c->mov++;   
        t->right = del_min(t->right, depth + 1, c);  
    }
    return rebalance(t, c);
}

AvlNode* avl_delete(AvlNode* root, int v, int* removed, Counters* c)
{
    *removed = 0;
    return del(root, v, 1, removed, c);
}

void avl_free(AvlNode* t)
{
    if (!t) return;
    avl_free(t->left); avl_free(t->right);
    free(t);
}

int avl_inorder(const AvlNode* t, int* out)
{
    if (!t) return 0;
    int k = avl_inorder(t->left, out);
    for (int i = 0; i < t->count; i++) out[k++] = t->key;
    return k + avl_inorder(t->right, out + k);
}

static int valid_rec(const AvlNode* t, long long lo, long long hi, int* height)
{
    if (!t) { *height = 0; return 1; }
    if (t->count < 1 || t->key <= lo || t->key >= hi) return 0;     
    int hl, hr;
    if (!valid_rec(t->left, lo, t->key, &hl) || !valid_rec(t->right, t->key, hi, &hr)) return 0;
    if (hl - hr > 1 || hr - hl > 1) return 0;
    *height = 1 + (hl > hr ? hl : hr);
    return *height == t->height;
}
int avl_valid(const AvlNode* t) { int hh; return valid_rec(t, -1, 0x7FFFFFFFLL, &hh); }
int avl_height(const AvlNode* t) { return h(t); }
long long avl_node_count(const AvlNode* t) { return t ? 1 + avl_node_count(t->left) + avl_node_count(t->right) : 0; }
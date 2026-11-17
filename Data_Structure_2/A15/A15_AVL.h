#ifndef A15_AVL_H
#define A15_AVL_H
#include "A15_Counters.h"

typedef struct AvlNode {
    int key;                 
    int count;               
    int height;              
    struct AvlNode* left, * right;
} AvlNode;

AvlNode* avl_insert(AvlNode* root, int v, Counters* c);

AvlNode* avl_delete(AvlNode* root, int v, int* removed, Counters* c);

void avl_free(AvlNode* root);

int  avl_inorder(const AvlNode* root, int* out);

int  avl_valid(const AvlNode* root);
int  avl_height(const AvlNode* root);
long long avl_node_count(const AvlNode* root);

#endif
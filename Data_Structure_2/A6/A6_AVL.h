#ifndef AVL_H
#define AVL_H

typedef struct AVLNode {
    int              data;
    int              height; 
    struct AVLNode* left;
    struct AVLNode* right;
} AVLNode;

AVLNode* avl_insert(AVLNode* root, int value, long* cmp_count, int* inserted);

int avl_search(const AVLNode* root, int target, long* cmp_count);

int avl_height(const AVLNode* root);

void avl_stats(const AVLNode* root, int* node_count, int* leaf_count);

void avl_free(AVLNode* root);

#endif
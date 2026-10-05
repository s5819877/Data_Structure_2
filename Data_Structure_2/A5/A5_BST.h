#ifndef BST_H
#define BST_H

typedef struct BSTNode {
    int              data;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

BSTNode* bst_insert(BSTNode* root, int value, long* cmp_count);

int bst_search(const BSTNode* root, int target, long* cmp_count);

void bst_stats(const BSTNode* root, int* node_count, int* leaf_count, int* height);

void bst_free(BSTNode* root);

#endif
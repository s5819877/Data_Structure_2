#ifndef BINTREE_H
#define BINTREE_H

#include <stddef.h>

typedef struct TreeNode {
    char             data;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* build_tree(const char* s, char* err, size_t errlen);

void print_tree(const TreeNode* root);

void preorder(const TreeNode* root);
void inorder(const TreeNode* root);
void postorder(const TreeNode* root);

long count_nodes(const TreeNode* root);

void free_tree(TreeNode* root);

#endif
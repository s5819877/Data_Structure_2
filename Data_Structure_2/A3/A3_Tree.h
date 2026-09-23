#ifndef BTREE_H
#define BTREE_H

typedef struct BTree BTree;

typedef enum {
    BT_OK = 0, 
    BT_ERR_EMPTY_TREE, 
    BT_ERR_NOT_FOUND, 
    BT_ERR_NOT_LEAF, 
    BT_ERR_FULL_CHILDREN, 
    BT_ERR_CHILD_OCCUPIED, 
    BT_ERR_INVALID_CHILD, 
    BT_ERR_DUP_SIBLING, 
    BT_ERR_ROOT_EXISTS, 
    BT_ERR_TREE_FULL, 
    BT_ERR_INVALID_DATA, 
    BT_ERR_INVALID_PATH, 
    BT_ERR_MEMORY 
} BTStatus;


BTree* create_btree(long size);

BTStatus insert_root(BTree* tree, char value);

BTStatus insert_child(BTree* tree, const char* parent_path, char child, char value);

BTStatus delete_node(BTree* tree, const char* leaf_path);

BTStatus update_value(BTree* tree, const char* node_path, char value);

BTStatus read_child(BTree* tree, const char* parent_path,
    char* left_out, char* right_out);

void print_btree(const BTree* tree);

void destroy_btree(BTree* tree);

int is_empty_btree(const BTree* tree);

const char* bt_status_message(BTStatus status);

#endif
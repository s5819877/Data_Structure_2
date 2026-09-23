#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "A3_Tree.h"

#define EMPTY '\0'
#define ABS_MAX_INDEX (1L << 24)

struct BTree {
    char* data; 
    long  capacity; 
    long  max_nodes; 
    long  count; 
};


static int exist(const BTree* tree, long idx)
{
    return (tree != NULL && idx >= 1 && idx < tree->capacity
        && tree->data[idx] != EMPTY);
}

static int ensure_capacity(BTree* tree, long idx)
{
    if (idx < 1 || idx >= ABS_MAX_INDEX) return 0;
    if (idx < tree->capacity) return 1;

    long newcap = tree->capacity;
    while (idx >= newcap) newcap *= 2;

    char* tmp = (char*)realloc(tree->data, (size_t)newcap * sizeof(char));
    if (!tmp) return 0;
    memset(tmp + tree->capacity, EMPTY, (size_t)(newcap - tree->capacity));
    tree->data = tmp;
    tree->capacity = newcap;
    return 1;
}

static BTStatus resolve_path(const BTree* tree, const char* path, long* out_idx)
{
    if (!path || path[0] != '/') return BT_ERR_INVALID_PATH;
    if (tree->count == 0) return BT_ERR_EMPTY_TREE;

    char buf[256];
    size_t len = strlen(path);
    if (len == 0 || len >= sizeof(buf)) return BT_ERR_INVALID_PATH;
    strcpy(buf, path);

    char* tok = strtok(buf, "/");
    if (!tok) return BT_ERR_INVALID_PATH;

    char c0 = (char)toupper((unsigned char)tok[0]);
    if (strlen(tok) != 1 || c0 < 'A' || c0 > 'Z') return BT_ERR_INVALID_PATH;
    if (!exist(tree, 1) || tree->data[1] != c0) return BT_ERR_NOT_FOUND;

    long idx = 1;
    tok = strtok(NULL, "/");
    while (tok) {
        char c = (char)toupper((unsigned char)tok[0]);
        if (strlen(tok) != 1 || c < 'A' || c > 'Z') return BT_ERR_INVALID_PATH;

        long l = idx * 2, r = idx * 2 + 1;
        if (exist(tree, l) && tree->data[l] == c) {
            idx = l;
        }
        else if (exist(tree, r) && tree->data[r] == c) {
            idx = r;
        }
        else {
            return BT_ERR_NOT_FOUND;
        }
        tok = strtok(NULL, "/");
    }

    *out_idx = idx;
    return BT_OK;
}

static void print_rec(const BTree* tree, long idx, int depth, const char* tag)
{
    if (!exist(tree, idx)) return;

    if (depth == 0) {
        printf("%c\n", tree->data[idx]);
    }
    else {
        for (int k = 0; k < depth - 1; k++) printf("    ");
        printf("+---%c%s\n", tree->data[idx], tag);
    }
    print_rec(tree, idx * 2, depth + 1, " (L)");
    print_rec(tree, idx * 2 + 1, depth + 1, " (R)");
}

BTree* create_btree(long size)
{
    BTree* t = (BTree*)malloc(sizeof(BTree));
    if (!t) return NULL;

    t->capacity = 2; 
    t->data = (char*)calloc((size_t)t->capacity, sizeof(char));
    if (!t->data) { free(t); return NULL; }

    t->max_nodes = (size > 0) ? size : -1; 
    t->count = 0;
    return t;
}

BTStatus insert_root(BTree* tree, char value)
{
    if (!tree) return BT_ERR_MEMORY;
    char v = (char)toupper((unsigned char)value);
    if (v < 'A' || v > 'Z') return BT_ERR_INVALID_DATA;
    if (tree->count != 0) return BT_ERR_ROOT_EXISTS;
    if (tree->max_nodes > 0 && tree->count >= tree->max_nodes) return BT_ERR_TREE_FULL;
    if (!ensure_capacity(tree, 1)) return BT_ERR_MEMORY;

    tree->data[1] = v;
    tree->count = 1;
    return BT_OK;
}

BTStatus insert_child(BTree* tree, const char* parent_path, char child, char value)
{
    if (!tree) return BT_ERR_MEMORY;

    char v = (char)toupper((unsigned char)value);
    if (v < 'A' || v > 'Z') return BT_ERR_INVALID_DATA;

    char c = (char)toupper((unsigned char)child);
    if (c != 'L' && c != 'R') return BT_ERR_INVALID_CHILD;

    long pidx;
    BTStatus st = resolve_path(tree, parent_path, &pidx);
    if (st != BT_OK) return st;

    long lidx = pidx * 2, ridx = pidx * 2 + 1;
    int has_l = exist(tree, lidx);
    int has_r = exist(tree, ridx);

    if (has_l && has_r) return BT_ERR_FULL_CHILDREN; 

    long target = (c == 'L') ? lidx : ridx;
    long sibling = (c == 'L') ? ridx : lidx;

    if ((c == 'L' && has_l) || (c == 'R' && has_r))
        return BT_ERR_CHILD_OCCUPIED; 

    if (exist(tree, sibling) && tree->data[sibling] == v)
        return BT_ERR_DUP_SIBLING; 

    if (tree->max_nodes > 0 && tree->count >= tree->max_nodes) return BT_ERR_TREE_FULL;
    if (!ensure_capacity(tree, target)) return BT_ERR_MEMORY;

    tree->data[target] = v;
    tree->count++;
    return BT_OK;
}

BTStatus delete_node(BTree* tree, const char* leaf_path)
{
    if (!tree) return BT_ERR_MEMORY;
    if (tree->count == 0) return BT_ERR_EMPTY_TREE;

    long idx;
    BTStatus st = resolve_path(tree, leaf_path, &idx);
    if (st != BT_OK) return st;

    if (exist(tree, idx * 2) || exist(tree, idx * 2 + 1))
        return BT_ERR_NOT_LEAF;

    tree->data[idx] = EMPTY;
    tree->count--;
    return BT_OK;
}

BTStatus update_value(BTree* tree, const char* node_path, char value)
{
    if (!tree) return BT_ERR_MEMORY;
    char v = (char)toupper((unsigned char)value);
    if (v < 'A' || v > 'Z') return BT_ERR_INVALID_DATA;
    if (tree->count == 0) return BT_ERR_EMPTY_TREE;

    long idx;
    BTStatus st = resolve_path(tree, node_path, &idx);
    if (st != BT_OK) return st;

    if (idx != 1) { 
        long sibling = (idx % 2 == 0) ? idx + 1 : idx - 1;
        if (exist(tree, sibling) && tree->data[sibling] == v)
            return BT_ERR_DUP_SIBLING;
    }

    tree->data[idx] = v;
    return BT_OK;
}

BTStatus read_child(BTree* tree, const char* parent_path,
    char* left_out, char* right_out)
{
    if (!tree) return BT_ERR_MEMORY;
    if (tree->count == 0) return BT_ERR_EMPTY_TREE;

    long idx;
    BTStatus st = resolve_path(tree, parent_path, &idx);
    if (st != BT_OK) return st;

    long l = idx * 2, r = idx * 2 + 1;
    *left_out = exist(tree, l) ? tree->data[l] : EMPTY;
    *right_out = exist(tree, r) ? tree->data[r] : EMPTY;
    return BT_OK;
}

void print_btree(const BTree* tree)
{
    if (!tree || tree->count == 0) {
        printf("(트리가 비어 있습니다)\n");
        return;
    }
    print_rec(tree, 1, 0, "");
}

void destroy_btree(BTree* tree)
{
    if (!tree) return;
    free(tree->data);
    free(tree);
}

int is_empty_btree(const BTree* tree)
{
    return (!tree || tree->count == 0);
}

const char* bt_status_message(BTStatus status)
{
    switch (status) {
    case BT_OK:                return "성공";
    case BT_ERR_EMPTY_TREE:    return "트리가 비어 있습니다.";
    case BT_ERR_NOT_FOUND:     return "해당 경로의 노드를 찾을 수 없습니다.";
    case BT_ERR_NOT_LEAF:      return "대상 노드가 단말 노드가 아닙니다.";
    case BT_ERR_FULL_CHILDREN: return "부모 노드가 이미 자식을 2개 가지고 있습니다.";
    case BT_ERR_CHILD_OCCUPIED:return "지정한 위치(L/R)에 이미 자식 노드가 있습니다.";
    case BT_ERR_INVALID_CHILD: return "child 인자는 L 또는 R 이어야 합니다.";
    case BT_ERR_DUP_SIBLING:   return "동일한 부모의 형제 노드와 데이터가 중복됩니다.";
    case BT_ERR_ROOT_EXISTS:   return "이미 루트 노드가 존재합니다.";
    case BT_ERR_TREE_FULL:     return "트리 용량(최대 노드 수)을 초과했습니다.";
    case BT_ERR_INVALID_DATA:  return "데이터는 영문 대문자 한 글자여야 합니다.";
    case BT_ERR_INVALID_PATH:  return "경로 형식이 올바르지 않습니다.";
    case BT_ERR_MEMORY:        return "메모리 할당에 실패했습니다.";
    default:                   return "알 수 없는 오류입니다.";
    }
}
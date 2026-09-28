#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "A4_TreeOrder.h"

#define INPUT_MAX (1 << 16)

int main(int argc, char* argv[])
{
    static char line[INPUT_MAX];
    char err[256];

    if (argc >= 2) {
        strncpy(line, argv[1], sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';
    }
    else {
        printf("이진트리의 괄호 표기법을 입력하세요 (예: A(B(D,E),C(,F))) : ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("[오류] 입력을 읽을 수 없습니다.\n");
            return 1;
        }
    }
    line[strcspn(line, "\r\n")] = '\0';

    TreeNode* root = build_tree(line, err, sizeof(err));
    if (!root) {
        printf("\n[오류] %s\n", err);
        printf("올바른 이진트리의 괄호 표기법이 아닙니다.\n");
        return 1;
    }

    printf("\n===== 입력된 이진트리 =====\n");
    printf("괄호 표기 : %s\n", line);
    printf("노드 수   : %ld\n\n", count_nodes(root));
    print_tree(root);

    printf("\n===== 순회 결과 (반복적 방법) =====\n");
    preorder(root);
    inorder(root);
    postorder(root);

    free_tree(root);
    return 0;
}
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "A3_Tree.h"

#define LINE_LEN 256
#define MAX_TOK  8
#define TREE_CAPACITY 100000

static void to_upper_str(const char* s, char* up, size_t buflen)
{
    size_t i = 0;
    for (; s[i] != '\0' && i + 1 < buflen; i++)
        up[i] = (char)toupper((unsigned char)s[i]);
    up[i] = '\0';
}

static char match_command(const char* tok)
{
    if (!tok || tok[0] == '\0') return 0;

    char up[32];
    to_upper_str(tok, up, sizeof(up));

    if (strlen(up) == 1) {
        if (strchr("IDURPQ", up[0])) return up[0];
        return 0;
    }
    if (strcmp(up, "INSERT") == 0) return 'I';
    if (strcmp(up, "DELETE") == 0) return 'D';
    if (strcmp(up, "UPDATE") == 0) return 'U';
    if (strcmp(up, "READ") == 0) return 'R';
    if (strcmp(up, "PRINT") == 0) return 'P';
    if (strcmp(up, "QUIT") == 0 || strcmp(up, "EXIT") == 0) return 'Q';
    return 0;
}

static char match_child(const char* tok)
{
    if (!tok || tok[0] == '\0') return 0;

    char up[32];
    to_upper_str(tok, up, sizeof(up));

    if (strlen(up) == 1) {
        if (up[0] == 'L' || up[0] == 'R') return up[0];
        return 0;
    }
    if (strcmp(up, "LEFT") == 0) return 'L';
    if (strcmp(up, "RIGHT") == 0) return 'R';
    return 0;
}

static void cmd_insert(BTree* tree, char* toks[], int ntok)
{
    BTStatus st;

    if (ntok == 3) {
        if (strcmp(toks[1], "/") != 0) {
            printf("[오류] 루트 생성은 'Insert / 데이터' 형식이어야 합니다.\n");
            return;
        }
        if (strlen(toks[2]) != 1) {
            printf("[오류] 데이터는 영문 대문자 한 글자여야 합니다.\n");
            return;
        }
        char v = (char)toupper((unsigned char)toks[2][0]);
        st = insert_root(tree, v);
        if (st == BT_OK)
            printf("루트 노드 '%c' 를 생성했습니다. (경로 : /%c)\n", v, v);
        else
            printf("[오류] %s\n", bt_status_message(st));

    }
    else if (ntok == 4) {
        char childc = match_child(toks[2]);
        if (!childc) {
            printf("[오류] child 인자는 L 또는 R 이어야 합니다: '%s'\n", toks[2]);
            return;
        }
        if (strlen(toks[3]) != 1) {
            printf("[오류] 데이터는 영문 대문자 한 글자여야 합니다.\n");
            return;
        }
        char v = (char)toupper((unsigned char)toks[3][0]);
        st = insert_child(tree, toks[1], childc, v);
        if (st == BT_OK)
            printf("%s%c%c 위치에 '%c' 노드를 추가했습니다.\n",
                toks[1], '/', v, v);
        else
            printf("[오류] %s\n", bt_status_message(st));

    }
    else {
        printf("[오류] Insert 명령의 인자 개수가 올바르지 않습니다.\n"
            "       사용법 : Insert / 데이터   또는   Insert 부모경로 L|R 데이터\n");
    }
}

static void cmd_delete(BTree* tree, char* toks[], int ntok)
{
    if (ntok != 2) {
        printf("[오류] Delete 명령의 인자 개수가 올바르지 않습니다.\n"
            "       사용법 : Delete 노드경로\n");
        return;
    }
    BTStatus st = delete_node(tree, toks[1]);
    if (st == BT_OK)
        printf("%s 노드를 삭제했습니다.\n", toks[1]);
    else
        printf("[오류] %s\n", bt_status_message(st));
}

static void cmd_update(BTree* tree, char* toks[], int ntok)
{
    if (ntok != 3) {
        printf("[오류] Update 명령의 인자 개수가 올바르지 않습니다.\n"
            "       사용법 : Update 노드경로 새데이터\n");
        return;
    }
    if (strlen(toks[2]) != 1) {
        printf("[오류] 데이터는 영문 대문자 한 글자여야 합니다.\n");
        return;
    }
    char v = (char)toupper((unsigned char)toks[2][0]);

    char newpath[LINE_LEN];
    strncpy(newpath, toks[1], sizeof(newpath) - 1);
    newpath[sizeof(newpath) - 1] = '\0';
    char* last_slash = strrchr(newpath, '/');
    if (last_slash) { last_slash[1] = v; last_slash[2] = '\0'; }

    BTStatus st = update_value(tree, toks[1], v);
    if (st == BT_OK)
        printf("%s 노드의 데이터를 '%c' (으)로 변경했습니다. (새 경로 : %s)\n",
            toks[1], v, newpath);
    else
        printf("[오류] %s\n", bt_status_message(st));
}

static void cmd_read(BTree* tree, char* toks[], int ntok)
{
    if (ntok != 2) {
        printf("[오류] Read 명령의 인자 개수가 올바르지 않습니다.\n"
            "       사용법 : Read 노드경로\n");
        return;
    }
    char l, r;
    BTStatus st = read_child(tree, toks[1], &l, &r);
    if (st != BT_OK) {
        printf("[오류] %s\n", bt_status_message(st));
        return;
    }
    if (l == '\0' && r == '\0') {
        printf("자식이 없습니다. (단말 노드)\n");
    }
    else if (l != '\0' && r != '\0') {
        printf("%c(L), %c(R)\n", l, r);
    }
    else if (l != '\0') {
        printf("%c(L)\n", l);
    }
    else {
        printf("%c(R)\n", r);
    }
}

static void cmd_print(const BTree* tree, int ntok)
{
    if (ntok != 1) {
        printf("[오류] Print 명령은 추가 인자를 받지 않습니다.\n");
        return;
    }
    print_btree(tree);
}

int main(void)
{
    printf("========================================================\n");
    printf("            과제-03 : 트리 조작 프로그램\n");
    printf("========================================================\n");
    printf("명령어 : Insert(I) / Delete(D) / Update(U) / Read(R) / Print(P)\n");
    printf("종료   : Quit(Q)\n");
    printf("예     : Insert / A\n");
    printf("         Insert /A L B\n");
    printf("         Read /A\n");
    printf("         Print\n");

    BTree* tree = create_btree(TREE_CAPACITY);
    if (!tree) {
        fprintf(stderr, "트리 생성에 실패했습니다. 프로그램을 종료합니다.\n");
        return 1;
    }

    char line[LINE_LEN];

    while (1) {
        printf("\n> ");
        if (!fgets(line, sizeof(line), stdin)) break; 

        line[strcspn(line, "\n")] = '\0'; 

        char* toks[MAX_TOK];
        int ntok = 0;
        char* t = strtok(line, " \t");
        while (t && ntok < MAX_TOK) {
            toks[ntok++] = t;
            t = strtok(NULL, " \t");
        }
        if (ntok == 0) continue; 

        char cmd = match_command(toks[0]);
        if (!cmd) {
            printf("[오류] 알 수 없는 명령어입니다: '%s'\n", toks[0]);
            continue;
        }

        switch (cmd) {
        case 'I': cmd_insert(tree, toks, ntok); break;
        case 'D': cmd_delete(tree, toks, ntok); break;
        case 'U': cmd_update(tree, toks, ntok); break;
        case 'R': cmd_read(tree, toks, ntok);   break;
        case 'P': cmd_print(tree, ntok);        break;
        case 'Q':
            goto quit;
        default:
            printf("[오류] 알 수 없는 명령어입니다.\n");
        }
    }

quit:
    destroy_btree(tree);
    printf("\n프로그램을 종료합니다.\n");
    return 0;
}
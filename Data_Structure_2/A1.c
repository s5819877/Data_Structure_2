#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _CRT_SECURE_NO_WARNINGS
#define MAXLEN 4096

static char buf[MAXLEN];
static int len;
static int pos;

static char node_stack[MAXLEN];
static int  count_stack[MAXLEN];
static int  ns = 0;

static int  out_depth[MAXLEN];
static char out_char[MAXLEN];
static int  out_n = 0;

static int total = 0;
static int leaf = 0;   
static int nonleaf = 0;
static int max_degree = 0;
static int height = 0;

static int  found_C = 0;
static int  parent_of_C_exists = 0;
static char parent_of_C = 0;
static char children_of_C[MAXLEN];
static int  children_of_C_n = 0;

typedef enum { EXPECT_NODE, EXPECT_COMMA_OR_CLOSE, EXPECT_END } State;
static State state = EXPECT_NODE;

static void error_exit(const char* msg) {
    printf("[오류] %s\n", msg);
    printf("올바른 트리의 괄호 표기법이 아닙니다. 프로그램을 종료합니다.\n");
    exit(1);
}

static void finish_node(char node_char) {
    if (ns > 0) {
        count_stack[ns - 1] += 1;
        char parent_char = node_stack[ns - 1];
        if (parent_char == 'C') {
            children_of_C[children_of_C_n++] = node_char;
        }
        state = EXPECT_COMMA_OR_CLOSE;
    }
    else {
        state = EXPECT_END;
    }
}

int main(void) {
    char raw[MAXLEN];

    printf("트리의 괄호 표기법을 입력하세요: ");
    if (!fgets(raw, sizeof(raw), stdin)) {
        error_exit("입력을 읽을 수 없습니다.");
    }

    int j = 0;
    for (int i = 0; raw[i] != '\0'; i++) {
        if (!isspace((unsigned char)raw[i])) {
            buf[j++] = raw[i];
        }
    }
    buf[j] = '\0';
    len = j;

    if (len == 0) {
        error_exit("입력이 비어 있습니다.");
    }

    pos = 0;
    state = EXPECT_NODE;

    while (1) {
        if (state == EXPECT_NODE) {
            if (pos >= len) error_exit("노드 이름이 와야 할 위치에서 입력이 끝났습니다.");

            char c = buf[pos];
            if (c < 'A' || c > 'Z') {
                error_exit("노드 이름은 영문 대문자 한 글자여야 합니다.");
            }

            total++;
            int depth = ns;
            if (depth > height) height = depth;
            out_depth[out_n] = depth;
            out_char[out_n] = c;
            out_n++;

            if (c == 'C') {
                found_C = 1;
                if (ns > 0) {
                    parent_of_C_exists = 1;
                    parent_of_C = node_stack[ns - 1];
                }
                else {
                    parent_of_C_exists = 0;
                }
            }

            pos++;
            if (pos < len && buf[pos] == '(') {
                nonleaf++;
                pos++;
                if (ns >= MAXLEN) error_exit("트리가 너무 깊습니다.");
                node_stack[ns] = c;
                count_stack[ns] = 0;
                ns++;
                state = EXPECT_NODE;
            }
            else {
                leaf++;
                finish_node(c);
            }
        }
        else if (state == EXPECT_COMMA_OR_CLOSE) {
            if (pos >= len) error_exit("괄호가 올바르게 닫히지 않았습니다.");

            char c = buf[pos];
            if (c == ',') {
                pos++;
                state = EXPECT_NODE;
            }
            else if (c == ')') {
                pos++;
                if (ns == 0) error_exit("괄호 짝이 맞지 않습니다.");
                char popped = node_stack[ns - 1];
                int deg = count_stack[ns - 1];
                if (deg > max_degree) max_degree = deg;
                ns--;
                finish_node(popped);
            }
            else {
                error_exit("',' 또는 ')' 가 와야 하는 위치에 다른 문자가 있습니다.");
            }
        }
        else {
            if (pos != len) {
                error_exit("트리 표현이 끝난 뒤에 불필요한 문자가 있습니다.");
            }
            break;
        }
    }

    printf("\n입력된 트리 : %s\n\n", buf);

    printf("전체 노드의 수   : %d\n", total);
    printf("단말 노드의 수   : %d\n", leaf);
    printf("비단말 노드의 수 : %d\n", nonleaf);
    printf("트리의 높이      : %d\n", height + 1);
    printf("트리의 차수      : %d\n", max_degree);

    if (!found_C) {
        printf("노드 C 는 이 트리에 존재하지 않습니다.\n");
    }
    else {
        if (parent_of_C_exists) {
            printf("C의 부모 노드    : %c\n", parent_of_C);
        }
        else {
            printf("C의 부모 노드    : 없음 (C가 루트 노드)\n");
        }

        printf("C의 자식 노드    : ");
        if (children_of_C_n == 0) {
            printf("없음\n");
        }
        else {
            for (int i = 0; i < children_of_C_n; i++) {
                printf("%c", children_of_C[i]);
                if (i != children_of_C_n - 1) printf(", ");
            }
            printf("\n");
        }
    }

    printf("\n트리 구조 (왼쪽으로 눕힌 형태) :\n");
    for (int i = 0; i < out_n; i++) {
        int d = out_depth[i];
        char c = out_char[i];
        if (d == 0) {
            printf("%c\n", c);
        }
        else {
            for (int k = 0; k < d - 1; k++) printf("    ");
            printf("+---%c\n", c);
        }
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum {
      TK_NUM = 256,
      TK_EOF,
      ND_NUM = 256,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

Token tokens[100];
int pos = 0;

typedef struct Node {
  int ty;
  struct Node *lsh;
  struct Node *rsh;
  int val;
} Node;

Node *expr();
Node *mul();
Node *term();
void error(int i);

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lsh = lhs;
  node->rsh = rhs;
  return node;
}

Node *new_node_num(int val)  {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *expr() {
  Node *lhs = mul();
  if(tokens[pos].ty == '+') {
    pos++;
    return new_node('+', lhs, expr());
  }
  if(tokens[pos].ty == '-') {
    pos++;
    return new_node('-', lhs, expr());
  }
  return lhs;
}

Node *mul() {
  Node *lhs = term();
  if(tokens[pos].ty == '*') {
    pos++;
    return new_node('*', lhs, mul());
  }
  if(tokens[pos].ty == '/') {
    pos++;
    return new_node('/', lhs, mul());
  }
  return lhs;
}

Node *term() {
  if(tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);
  if(tokens[pos].ty == '(') {
    pos++;
    Node *node = expr();
    if (tokens[pos].ty != ')')
      error(pos);
    pos++;
    return node;
  }
  error(pos);
}

void tokenize(char *p) {
  int i = 0;

  while (*p)
    {
      if (isspace(*p)) {
          p++;
          continue;
        }

      if (*p == '+' || *p == '-' || *p == '/' || *p == '*' || *p == '(' || *p == ')') {
          tokens[i].ty = *p;
          tokens[i].input = p;
          i++;
          p++;
          continue;
        }

      if (isdigit(*p)) {
          tokens[i].ty = TK_NUM;
          tokens[i].input = p;
          tokens[i].val = strtol(p, &p, 10);
          i++;
          continue;
        }

      fprintf(stderr, "トークナイズできません: %s\n", p);
      exit(1);
    }
  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

void error(int i) {
  fprintf(stderr, "予期せぬトークンです: %s\n", tokens[i].input);
  exit(1);
}

void gen(Node *node) {
  if(node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lsh);
  gen(node->rsh);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    break;
  }
  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
      fprintf(stderr, "引数の個数が正しくありません");
      return 1;
  }
  tokenize(argv[1]);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global _main\n");
  printf("_main:\n");
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

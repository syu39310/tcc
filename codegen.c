#include "tcc.h"

static int endIfCnt;
static int elseCnt;
static int beginCnt;
static int endCnt;

void debugComment(char *val) {
  printf("#%s\n", val);
}

char *getTokenStr(Token *token) {
  char *ret;
  strncpy(ret, token->str, token->len);
  ret[token->len] = '\0';
  return ret;
}

void gen_lvel(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->kind == ND_BLOCK) {
    Node *cur = node->body;
    while(cur) {
      gen(cur);
      printf("  pop rax\n");
      cur = cur->next;
    }
    return;
  }

  if (node->kind == ND_FUNCALL) {
    if (node->args[0]) 
      printf("  mov rdi, %d\n", node->args[0]->val);
    if (node->args[1]) 
      printf("  mov rsi, %d\n", node->args[1]->val);
    if (node->args[2]) 
      printf("  mov rdx, %d\n", node->args[2]->val);
    if (node->args[3]) 
      printf("  mov rcx, %d\n", node->args[3]->val);
    if (node->args[4]) 
      printf("  mov r8, %d\n", node->args[4]->val);
    if (node->args[5]) 
      printf("  mov r9, %d\n", node->args[5]->val);

    printf("  call %s\n", getTokenStr(node->token));
    return;
  }

  if (node->kind == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  // 制御文
  switch (node->kind) {
    case ND_IF:
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lelse%d\n", elseCnt);
      gen(node->then);
      printf("  jmp  .LendIf%d\n", endIfCnt);
      printf(".Lelse%d:\n", elseCnt++);
      if (node->els) {
        gen(node->els);
      }
      printf(".LendIf%d:\n", endIfCnt++);
      return;
    case ND_FOR:
      if (node->init) {
        gen(node->init);
      }
      printf(".Lbegin%d:\n", beginCnt);
      if (node->cond) {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%d\n", endCnt);
      }
      if (node->then) {
        gen(node->then);
      }
      if (node->inc) {
        gen(node->inc);
      }
      printf("  jmp .Lbegin%d\n", beginCnt++);
      printf(".Lend%d:\n", endCnt++);
      return;
  }

  // 変数
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lvel(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lvel(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  default:
    break;
  }

  printf("  push rax\n");
}

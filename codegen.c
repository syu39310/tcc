#include "tcc.h"

static int labelseq;
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

void debug_comment(char *val) {
  printf("#%s\n", val);
}

void gen_lvel(Node *node) {
  if (node->kind != ND_VAR)
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
    printf("  push rax\n");
    int nargs = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      printf("  push %d\n", arg->val);
      nargs++;
    }
    for (int i = 1; i <= nargs; i++) {
      printf("  pop rax\n");
      printf("  mov %s, rax\n", argreg[nargs - i]);
    }
    printf("  pop rax\n");

    int seq = labelseq++;
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    printf("  jnz .L.call.%d\n", seq);
    printf("  mov rax, 0\n");
    printf("  call %s\n", get_token_str(node->token));
    printf("  jmp .L.end.%d\n", seq);
    printf(".L.call.%d:\n", seq);
    printf("  sub rsp, 8\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", get_token_str(node->token));
    printf("  add rsp, 8\n");
    printf(".L.end.%d:\n", seq);
    printf("  push rax\n");
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
      {
        int seqElse = labelseq++;
        int seqEndIf = labelseq++;
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", seqElse);
        gen(node->then);
        printf("  jmp  .LendIf%d\n", seqEndIf);
        printf(".Lelse%d:\n", seqElse++);
        if (node->els) {
          gen(node->els);
        }
        printf(".LendIf%d:\n", seqEndIf);
        return;
      }
    case ND_FOR:
      {
        int seqBegin = labelseq++;
        int seqEnd = labelseq++;
        if (node->init) {
          gen(node->init);
        }
        printf(".Lbegin%d:\n", seqBegin);
        if (node->cond) {
          gen(node->cond);
          printf("  pop rax\n");
          printf("  cmp rax, 0\n");
          printf("  je .Lend%d\n", seqEnd);
        }
        if (node->then) {
          gen(node->then);
        }
        if (node->inc) {
          gen(node->inc);
        }
        printf("  jmp .Lbegin%d\n", seqBegin);
        printf(".Lend%d:\n", seqEnd);
        return;
      }
  }

  // 変数
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_VAR:
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

#include "tcc.h"

static int top;
static int labelseq;
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

void debug_comment(char *val) {
  printf("#%s\n", val);
}

//static char *reg(int idx) {
//  static char *r[] = {"%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
//  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
//    error("register out of range: %d", idx);
//  return r[idx];
//}

void gen_addr(Node *node) {
  if (node->kind == ND_VAR) {
    printf("  lea rax, [rbp-%d]\n", node->offset);
    printf("  push rax\n");
    return;
  }

  error("not an lvalue");
}

void load() {
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}
void store() {
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}

void gen_expr(Node *node);
void gen_stmt(Node *node);
void code_gen(Function *func) {
  // 関数の定義
  printf(".global %s\n", func->name);
  printf("%s:\n", func->name);

  // Prologue. %r12-15 are callee-saved registers.
  printf("  push %%rbp\n");
  printf("  mov %%rbp, %%rsp\n");
  printf("  sub %%rsp, %d\n", func->stack_size);
  // printf("  mov %%r12, -8(%%rbp)\n");
  // printf("  mov %%r13, -16(%%rbp)\n");
  // printf("  mov %%r14, -24(%%rbp)\n");
  // printf("  mov %%r15, -32(%%rbp)\n");
  
  int i=0;
  for (Var *var = func->params; var; var = var->next)
    i++;
  for (Var *var = func->params; var; var = var->next)
    printf("  mov %s, -%d(rbp)\n", argreg[--i], var->offset);

  Node *node = func->body;
  while (node) {
    debug_print("body start");
    gen_stmt(node);
    debug_print("body end");

    // 式の評価結果としてスタックに一つの値が残っているはずなので、
    // スタックが煽れない様にポップしておく
    printf("  pop rax\n");
    node = node->next;
  }
  
  // Epilogue
  printf(".L.return.%s:\n", func->name);
  // printf("  mov -8(%%rbp), %%r12\n");
  // printf("  mov -16(%%rbp), %%r13\n");
  // printf("  mov -24(%%rbp), %%r14\n");
  // printf("  mov -32(%%rbp), %%r15\n");
  printf("  mov %%rsp, %%rbp\n");
  printf("  pop %%rbp\n");
  printf("  ret\n");
}

void gen_stmt(Node *node) {
  debug_node(node);
  // 制御文
  switch (node->kind) {
    case ND_BLOCK: 
      {
        debug_print("block start");
        Node *cur = node->body;
        while(cur) {
          gen_stmt(cur);
          printf("  pop rax\n");
          cur = cur->next;
        }
        debug_print("block end");
        return;
      }
    case ND_IF:
      {
        debug_print("if start");
        int seqElse = labelseq++;
        int seqEndIf = labelseq++;
        gen_expr(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lelse%d\n", seqElse);
        gen_stmt(node->then);
        printf("  jmp  .LendIf%d\n", seqEndIf);
        printf(".Lelse%d:\n", seqElse++);
        if (node->els) {
          gen_expr(node->els);
        }
        printf(".LendIf%d:\n", seqEndIf);
        debug_print("if end");
        return;
      }
    case ND_FOR:
      {
        debug_print("for start");
        int seqBegin = labelseq++;
        int seqEnd = labelseq++;
        if (node->init) {
          gen_expr(node->init);
        }
        printf(".Lbegin%d:\n", seqBegin);
        if (node->cond) {
          gen_expr(node->cond);
          printf("  pop rax\n");
          printf("  cmp rax, 0\n");
          printf("  je .Lend%d\n", seqEnd);
        }
        if (node->then) {
          gen_stmt(node->then);
        }
        if (node->inc) {
          gen_expr(node->inc);
        }
        printf("  jmp .Lbegin%d\n", seqBegin);
        printf(".Lend%d:\n", seqEnd);
        debug_print("for end");
        return;
      }
    default:
    {
      debug_print("default");
      gen_expr(node);
      break;
    }
  }
  debug_print("gen_stmt end");
}

void gen_expr(Node *node) {
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
    gen_expr(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  // 変数
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_VAR:
      gen_addr(node);
      load();
      return;
    case ND_ASSIGN:
      gen_addr(node->lhs);
      gen_expr(node->rhs);
      store();
      return;
  }
  gen_expr(node->lhs);
  gen_expr(node->rhs);

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

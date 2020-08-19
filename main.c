#include "tcc.h"

// 入力プログラム(エラー表示用)
char *user_input;

void debug_print(char *val) {
  fprintf(stderr, "###%s\n", val);
}

void debug_token(Token *tok) {
  if (tok) {
    fprintf(stderr, "###token: kind[%d], val[%d], str[%s], len[%d]\n", tok->kind, tok->val, tok->str, tok->len);
    //fprintf(stderr, "###token: kind[%d], val[%d], str[%s], len[%d]\n", tok->kind, tok->val, get_token_str(tok), tok->len);
  } else {
    fprintf(stderr, "###Token is NULL\n");
  }
}

void debug_func(Function *func) {
  if (func) {
    fprintf(stderr, "###func: name[%s], stack_size[%d]\n", func->name, func->stack_size);
    fprintf(stderr, "####params\n");
    for (Var *var = func->params; var; var = var->next)
      debug_var(var);
    fprintf(stderr, "####locals\n");
    for (Var *var = func->locals; var; var = var->next)
      debug_var(var);
  } else {
    fprintf(stderr, "###Function is NULL\n");
  }
}

void debug_node(Node *node) {
  if (node) {
    fprintf(stderr, "###node: kind[%d], val[%d], offset[%d]\n", node->kind, node->val, node->offset);
  } else {
    fprintf(stderr, "###Node is NULL\n");
  }
}

void debug_var(Var *var) {
if (var) {
    fprintf(stderr, "###var: name[%s], len[%d], offset[%d]\n", get_var_name(var), var->len, var->offset);
  } else {
    fprintf(stderr, "###Var is NULL\n");
  }
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

char *get_str(char *str, int len) {
  char tmp[len];
  strncpy(tmp, str, len);
  tmp[len] = '\0';
  char *ret;
  ret = tmp;
  return ret;
}

char *get_token_str(Token *token) {
  if (token->len ==1) {
    return get_str(token->str, token->len);
  }
  char *ret;
  strncpy(ret, token->str, token->len);
  ret[token->len] = '\0';
  return ret;
}

char *get_var_name(Var *var) {
  return get_str(var->name, var->len);
}

// Round up `n` to the nearest multiple of `align`. For instance,
// align_to(5, 8) returns 8 and align_to(11, 8) returns 16.
static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません。 [%d]%s, %s, %s", argc, argv[0], argv[1], argv[2]);
    return 1;
  }

  // トークナイズ & パース
  // 結果はcodeに保存される
  user_input = argv[1];
  Token *token = tokenize(user_input);
  Function *func = parse(token);
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  // Assign offsets to local variables.

  for (Function *fn = func; fn; fn = fn->next) {
    //int offset = 32; // 32 for callee-saved registers
    int offset = 0;
    for (Var *var = fn->locals; var; var = var->next) {
      offset += 8;
      var->offset = offset;
    }
    fn->stack_size = align_to(offset, 16);
  }

  // 抽象構文木を降りながらコード生成
  while (func) {
    code_gen(func);
    func = func->next;
  }
  
  return 0;
}
#include "tcc.h"

// 入力プログラム(エラー表示用)
char *user_input;

void debug_print(char *val) {
  fprintf(stderr, "###%s\n", val);
}

void debug_token(Token *tok) {
  if (tok) {
    fprintf(stderr, "###token: kind[%d], val[%d], str[%s], len[%d]\n", tok->kind, tok->val, tok->str, tok->len);
  } else {
    fprintf(stderr, "###Token is NULL\n");
  }
}

void debug_func(Function *func) {
  if (func) {
    fprintf(stderr, "###func: name[%s]\n", func->name);
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

char *get_token_str(Token *token) {
  char *ret;
  strncpy(ret, token->str, token->len);
  ret[token->len] = '\0';
  return ret;
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
  
  // 抽象構文木を降りながらコード生成
  while (func) {
    // プロローグ
    // 関数の定義
    printf(".global %s\n", func->name);
    printf("%s:\n", func->name);
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");
    Node *node = func->body;
    while (node) {
      gen(node);

      // 式の評価結果としてスタックに一つの値が残っているはずなので、
      // スタックが煽れない様にポップしておく
      printf("  pop rax\n");
      node = node->next;
    }
    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    func = func->next;
  }
  
  return 0;
}
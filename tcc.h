#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Token Token;
typedef struct Function Function;
typedef struct Node Node;
typedef struct Var Var;

// ローカル変数の型
struct Var {
  Var *next; // 次の変数かNull
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,     //00 +
  ND_SUB,     //01 -
  ND_MUL,     //02 *
  ND_DIV,     //03 /
  ND_EQ,      //04 ==
  ND_NE,      //05 !=
  ND_LT,      //06 <
  ND_LE,      //07 <=
  ND_NUM,     //08 整数
  ND_ASSIGN,  //09 =
  ND_RETURN,  //10 return文
  ND_VAR,     //11 変数
  ND_IF,      //12 IF
  ND_FOR,     //13 FOR, WHILE
  ND_BLOCK,   //14 BLOCK
  ND_FUNCALL, //15 関数呼び出し
  ND_EOF,     //16 終端ノード
} NodeKind;

struct Function {
  Function *next;
  char *name;
  Var *params;

  Node *body;
  Var *locals;
  int stack_size;
};

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  Token *token;  // token
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_VARの場合のみ使う
  // "if" or "for" statement
  Node *cond;    // condition
  Node *then;    // if、while、for 中身
  Node *els;     // else
  Node *init;    // if 初期化
  Node *inc;     // if インクリメント
  Node *body;    // block 中身
  Node *args;   // function 引数
  Node *next;    // 次のノード(次行、ループの中)
};

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_RETURN,   // return文
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// main.c
void debug_print(char *val);
void debug_token(Token *tok);
void debug_func(Function *func);
void debug_node(Node *node);
void debug_var(Var *var);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
char *get_token_str(Token *token);
char *get_var_name(Var *var);

// tokenizer.c
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize(char *user_input);

// parser.c
Function *parse(Token *token);

// codegen.c
void code_gen(Function *func);
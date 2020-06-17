#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNull
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 整数
  ND_ASSIGN, // =
  ND_RETURN, // return文
  ND_LVAR, // ローカル変数
  ND_IF, // IF
  ND_FOR, // FOR, WHILE
  ND_EOF,  // 終端ノード
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
  // "if" or "for" statement
  Node *cond;    // condition
  Node *then;
  Node *els;     // else
  Node *init;
  Node *inc;
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

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// main.c
void debugPrint(char *val);
void debugToken(Token *tok);
void debugNode(Node *node);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

// tokenizer.c
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize(char *user_input);

// parser.c
Node *program(Token *token);

// codegen.c
void gen(Node *node);
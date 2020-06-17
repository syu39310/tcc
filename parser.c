#include "tcc.h"

// 現在着目しているトークン
Token *token;
// ローカル変数
LVar *locals;

// 次のトークンが期待値のときには、真を返す。それ以外の場合には偽を返す。
bool equal(Token *tok, char *op) {
  return strlen(op) == tok->len &&
          !strncmp(tok->str, op, tok->len);
}

// 次のトークンが期待値のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合にはエラー。
Token *skip(char *op) {
  if (equal(token, op)) {
    token = token->next;
    return token;
  }
  error_tok(token, "expected '%s'", op);
}

// 次のトークンが期待値のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (equal(token, op)) {
    token = token->next;
    return true;
  }
  return false;
}

// 次のトークンがreturn文のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume_return() {
  if (token->kind != TK_RETURN) 
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している識別子のときには、現在のトークン
// 次トークンを返す。それ以外の場合にはNULLを返す。
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *result = token;
  token = token->next;
  return result;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// グローバルに置いているcode[]にparse結果を設定する。
// program = stmt*
Node *program(Token *argToken) {
  token = argToken;
  int i = 0;
  
  Node head;
  head.next = NULL;
  Node *cur = &head;
  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }

  return head.next;
}

// stmt    = expr ";"
//         | "if" "(" expr ")" stmt ("else" stmt)?
//         | "while" "(" expr ")" stmt
//         | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//         | ...
Node *stmt() {
    if (consume("if")) {
      Node *node = new_node(ND_IF);
      skip("(");
      node->cond = expr();
      skip(")");
      node->then = stmt();
      if (consume("else")) {
        node->els = stmt();
      }
      return node;
    } 
    if(consume("while")) {
      Node *node = new_node(ND_FOR);
      skip("(");
      node->cond = expr();
      skip(")");
      node->then = stmt();
      return node;
    } 
    if(consume("for")) {
      Node *node = new_node(ND_FOR);
      skip("(");
      if (!consume(";")) {
        node->init = expr();
        skip(";");
      }
      if (!consume(";")) {
        node->cond = expr();
        skip(";");
      }
      if (!consume(")")) {
        node->inc = expr();
        skip(")");
      }
      node->then = stmt();
      
      return node;
    } 

    if (consume_return()) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_RETURN;
      node->lhs = expr();
      skip(";");
      return node;
    } 
    Node *node = expr();
    skip(";");
    
    return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), primary());
  return primary();
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    skip(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_LVAR;

      LVar *lvar = find_lvar(tok);
      if (lvar) {
        node->offset = lvar->offset;
      } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = locals==NULL ? 0 : locals->offset+8;
        node->offset = lvar->offset;
        locals = lvar;
      }
      
      return node;
  }

  // そうでなければ数値のはず
  return new_num(expect_number());
}

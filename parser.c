#include "tcc.h"

// 現在着目しているトークン
Token *token;
// ローカル変数
Var *locals;

// 次のトークンが期待値のときには、真を返す。それ以外の場合には偽を返す。
bool equal(Token *tok, char *op) {
  return strlen(op) == tok->len &&
          !strncmp(tok->str, op, tok->len);
}

// 次のトークンが期待値のときには、トークンを1つ読み進めて
// トークンを返す。それ以外の場合にはエラー。
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

// 次のトークンが期待しているトークン種類のときには、トークンを読み進めて現在のトークンを返す。
// それ以外の場合にはNULLを返す。
Token *consume_kind(TokenKind kind) {
  if (token->kind != kind) 
    return NULL;
  Token *result = token;
  token = token->next;
  return result;
}

// 次のトークンが期待の種類の場合、トークンを1つ読み進めてそのトークンを返す。
// それ以外の場合にはエラーを報告する。
Token *expect_kind(TokenKind kind, char *errArg) {
  if (token->kind != kind)
    error_at(token->str, "%sではありません", errArg);
  Token *result = token;
  token = token->next;
  return result;
}

int expect_number() {
  return expect_kind(TK_NUM, "数")->val;
}

char *expect_ident() {
  return get_token_str(expect_kind(TK_IDENT, "識別子"));
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->token = tok;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
  node->val = val;
  return node;
}

Function *funcdef();
Node *compound_stmt();
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
// parse = stmt*
Function *parse(Token *argToken) {
  token = argToken;

  Function head;
  head.next = NULL;
  Function *cur = &head;
  while (!at_eof()) {
    cur->next = funcdef();
    cur = cur->next;
  }

  return head.next;
}

// funcdef = "int" ident() compound_stmt
Function *funcdef() {
  skip("int");
  Function *func = calloc(1, sizeof(Function));
  func->name = expect_ident();
  skip("(");
  //func->params;
  skip(")");
  func->body = compound_stmt();
  //func->locals;
  //func->stack_size;

  return func;
}

// compound_stmt = {stmt*}
Node *compound_stmt() {
  skip("{");
  Node head;
  head.next = NULL;
  Node *cur = &head;
  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }

  return head.next;
}
// stmt    = expr ";"
//         | "{" stmt* "}"
//         | "if" "(" expr ")" stmt ("else" stmt)?
//         | "while" "(" expr ")" stmt
//         | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//         | "return" expr ";"
Node *stmt() {
    if (consume("{")) {
      Node head;
      head.next = NULL;
      Node *cur = &head;
      while(!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
      }
      Node *node = new_node(ND_BLOCK, token);
      node->body = head.next;
      return node;
    }
    // 制御構文
    if (consume("if")) {
      Node *node = new_node(ND_IF, token);
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
      Node *node = new_node(ND_FOR, token);
      skip("(");
      node->cond = expr();
      skip(")");
      node->then = stmt();
      return node;
    } 
    if(consume("for")) {
      Node *node = new_node(ND_FOR, token);
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
    // return文
    if (consume_kind(TK_RETURN)) {
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
        node = new_binary(ND_ASSIGN, node, assign(), token);
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational(), token);
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational(), token);
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add(), token);
    else if (consume("<="))
      node = new_binary(ND_LE, node, add(), token);
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node, token);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node, token);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul(), token);
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul(), token);
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary(), token);
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary(), token);
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0, token), primary(), token);
  return primary();
}

// num
//  | ident ("(" num? ("," num)?* ")")?
//  | "(" expr ")"
Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    skip(")");
    return node;
  }

  Token *tok = consume_kind(TK_IDENT);
  if (tok) {
      if (consume("(")) {
        Node head;
        head.next = NULL;
        Node *cur = &head;
        for (int i=0; i <= 6; i++) {
          if (consume(")")) {
            break;
          } if (i != 0) {
            skip(",");
          }
          cur->next = new_num(expect_number(), token);
          cur = cur->next;
        }
        Node *node = new_node(ND_FUNCALL, tok);
        node->args = head.next;
        return node;
      }

      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_VAR;

      Var *var = find_var(tok);
      if (var) {
        node->offset = var->offset;
      } else {
        var = calloc(1, sizeof(Var));
        var->next = locals;
        var->name = tok->str;
        var->len = tok->len;
        var->offset = locals==NULL ? 0 : locals->offset+8;
        node->offset = var->offset;
        locals = var;
      }
      
      return node;
  }

  // そうでなければ数値のはず
  return new_num(expect_number(), token);
}

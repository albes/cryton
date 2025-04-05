#ifndef cryton_parser_h
#define cryton_parser_h

#include "value.h"
#include "bigint.h"
#include "scanner.h"

typedef enum {
    EXPR_BINARY, EXPR_UNARY,
    EXPR_NUMBER, EXPR_VAR
} ExprType;

typedef struct {
    ExprType type;
} Expr;

typedef struct
{
    ExprType type;
    TokenType operator;
    Expr* left;
    Expr* right;
} ExprBinary;

typedef struct {
    ExprType type;
    TokenType operator;
    Expr* right;
} ExprUnary;

typedef struct {
    ExprType type;
    BigInt value;
} ExprNumber;

typedef struct {
    ExprType type;
    ObjString* name;
} ExprVar;

typedef enum {
    STMT_ASSIGN, STMT_PRINT,
    STMT_IF, STMT_WHILE
} StmtType;

typedef struct Stmt Stmt;

struct Stmt {
    StmtType type;
    Stmt* next;
};

typedef struct {
    Stmt stmt;
    ExprVar* left;
    Expr* right;
} StmtAssign;

typedef struct {
    Stmt stmt;
    Expr* expr;
} StmtPrint;

typedef struct {
    Stmt stmt;
    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;
} StmtIf;

typedef struct {
    Stmt stmt;
    Expr* condition;
    Stmt* body;
} StmtWhile;

bool parse(const char* source, Stmt** stmts);
void freeAST(Stmt* stmts);

#endif

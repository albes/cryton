#ifndef cryton_parser_h
#define cryton_parser_h

#include "value.h"
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
    int value;
} ExprNumber;

typedef struct {
    ExprType type;
    ObjString* name;
} ExprVar;

typedef enum {
    STMT_SEQUENCE,
    STMT_ASSIGN, STMT_PRINT,
    STMT_IF, STMT_WHILE
} StmtType;

typedef struct {
    StmtType type;
} Stmt;

typedef struct StmtSeq StmtSeq;

struct StmtSeq {
    StmtType type;
    Stmt* stmt;
    StmtSeq* next;
};

typedef struct {
    StmtType type;
    ExprVar* left;
    Expr* right;
} StmtAssign;

typedef struct {
    StmtType type;
    Expr* expr;
} StmtPrint;

typedef struct {
    StmtType type;
    Expr* condition;
    StmtSeq* thenBranch;
    Stmt* elseBranch;
} StmtIf;

typedef struct {
    StmtType type;
    Expr* condition;
    StmtSeq* body;
} StmtWhile;

bool parse(const char* source, Stmt** stmt);

#endif

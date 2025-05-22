#ifndef cryton_parser_h
#define cryton_parser_h

#include "value.h"
#include "bigint.h"
#include "scanner.h"

typedef enum {
    EXPR_BINARY, EXPR_UNARY,
    EXPR_NUMBER, EXPR_VAR,
    EXPR_IN, EXPR_MORPHISM,
    EXPR_CAT_INIT
} ExprType;


typedef struct Expr{
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
    STMT_IF, STMT_WHILE,
    STMT_CAT
} StmtType;

typedef struct Stmt Stmt;

struct Stmt {
    StmtType type;
    Stmt* next;
};

typedef struct {
    Expr expr;
    ObjString* callee;   // name of the category (e.g. p)
    Expr** args;       // arguments like (a + 1), 5, etc.
    int argCount;
} ExprCatInit;

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

typedef struct {
    Stmt stmt;
    ObjString* name;
    ObjString** params;
    int paramCount;
    TmplObjects objects;
    TmplHomSet homset;
} StmtCat;

bool parse(const char* source, Stmt** stmts);
void freeAST(Stmt* stmts);

typedef struct {
    ExprType type;
    Expr* from;
    Expr* to;
} ExprMorphism;

typedef struct {
    Expr expr;
    Expr* element;
    ExprVar* name;     // change to type to smth else in the future
} ExprIn;

#endif

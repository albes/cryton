#ifndef cryton_interpreter_h
#define cryton_interpreter_h

#include "bigint.h"
#include "table.h"
#include "parser.h"

typedef struct {
    Table strings;
} Interp;

extern Interp interp;

#define MAX_CATEGORIES 256

typedef struct {
    RuntimeCategory categories[MAX_CATEGORIES];
    int categoryCount;
} Runtime;

void initInterp();
void freeInterp();
BigInt interpretExpr(Expr* expr);
void interpretStmt(Stmt* stmt);
void interpret(Stmt* stmts);

#endif

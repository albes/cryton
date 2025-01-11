#ifndef cryton_interpreter_h
#define cryton_interpreter_h

#include "table.h"
#include "parser.h"

typedef struct {
    Table strings;
} Interp;

extern Interp interp;

void initInterp();
void freeInterp();
int interpretExpr(Expr* expr);
void interpretStmt(Stmt* stmt);
void interpret(Stmt* stmts);

#endif

#include <stdio.h>

#include "common.h"
#include "table.h"
#include "interpreter.h"

Interp interp;

void initInterp() {
    initTable(&interp.strings);
}

void freeInterp() {
    freeTable(&interp.strings);
}

int interpretBinary(ExprBinary* expr) {
    switch (expr->operator) {
        case TOKEN_PLUS        : return interpretExpr(expr->left) +  interpretExpr(expr->right);
        case TOKEN_MINUS       : return interpretExpr(expr->left) -  interpretExpr(expr->right);
        case TOKEN_LESS        : return interpretExpr(expr->left) <  interpretExpr(expr->right);
        case TOKEN_GREATER     : return interpretExpr(expr->left) >  interpretExpr(expr->right);
        case TOKEN_EQUAL_EQUAL : return interpretExpr(expr->left) == interpretExpr(expr->right);
        case TOKEN_BANG_EQUAL  : return interpretExpr(expr->left) != interpretExpr(expr->right);
        case TOKEN_AND         : return interpretExpr(expr->left) && interpretExpr(expr->right);
        case TOKEN_OR          : return interpretExpr(expr->left) || interpretExpr(expr->right);
    }

    return 0;
}

int interpretUnary(ExprUnary* expr) {
    switch (expr->operator) {
        case TOKEN_MINUS : return -interpretExpr(expr->right);
        case TOKEN_NOT   : return !interpretExpr(expr->right);
    }

    return 0;
}

int interpretNumber(ExprNumber* expr) {
    return expr->value;
}

// TODO: report an error if variable is uninitialized
int interpretVar(ExprVar* expr) {
    Value value = { VALUE_NUMBER, 0 };
    tableGet(&interp.strings, expr->name, &value);
    return value.number;
}

int interpretExpr(Expr* expr) {
    switch (expr->type) {
        case EXPR_BINARY : return interpretBinary((ExprBinary*)expr);
        case EXPR_UNARY  : return interpretUnary((ExprUnary*)expr);
        case EXPR_NUMBER : return interpretNumber((ExprNumber*)expr);
        case EXPR_VAR    : return interpretVar((ExprVar*)expr);
    }

    return 0;
}

void interpretStmt(Stmt* stmt);

void interpretAssign(StmtAssign* stmt) {
    ExprVar* exprVar = (ExprVar*)stmt->left;
    ObjString* varName = exprVar->name;
    int value = interpretExpr(stmt->right);
    tableSet(&interp.strings, varName, (Value){ VALUE_NUMBER, value });
}

void interpretPrint(StmtPrint* stmt) {
    printf("%d\n", interpretExpr(stmt->expr));
}

void interpretIf(StmtIf* stmt) {
    if (interpretExpr(stmt->condition)) {
        interpret(stmt->thenBranch);
    } else {
        interpret(stmt->elseBranch);
    }
}

void interpretWhile(StmtWhile* stmt) {
    while (interpretExpr(stmt->condition)) {
        interpret(stmt->body);
    }
}

void interpretStmt(Stmt* stmt) {
    if (stmt == NULL) return;

    switch (stmt->type) {
        case STMT_ASSIGN: interpretAssign((StmtAssign*)stmt); break;
        case STMT_PRINT: interpretPrint((StmtPrint*)stmt); break;
        case STMT_IF: interpretIf((StmtIf*)stmt); break;
        case STMT_WHILE: interpretWhile((StmtWhile*)stmt); break;
    }
}

void interpret(Stmt* stmts) {
    while (stmts != NULL) {
        interpretStmt(stmts);
        stmts = stmts->next;
    }
}

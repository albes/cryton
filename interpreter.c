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
        case TOKEN_PLUS        : return interpret(expr->left) +  interpret(expr->right);
        case TOKEN_MINUS       : return interpret(expr->left) -  interpret(expr->right);
        case TOKEN_LESS        : return interpret(expr->left) <  interpret(expr->right);
        case TOKEN_GREATER     : return interpret(expr->left) >  interpret(expr->right);
        case TOKEN_EQUAL_EQUAL : return interpret(expr->left) == interpret(expr->right);
        case TOKEN_BANG_EQUAL  : return interpret(expr->left) != interpret(expr->right);
        case TOKEN_AND         : return interpret(expr->left) && interpret(expr->right);
        case TOKEN_OR          : return interpret(expr->left) || interpret(expr->right);
    }

    return 0;
}

int interpretUnary(ExprUnary* expr) {
    switch (expr->operator) {
        case TOKEN_MINUS : return -interpret(expr->right);
        case TOKEN_NOT   : return !interpret(expr->right);
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

int interpret(Expr* expr) {
    switch (expr->type) {
        case EXPR_BINARY : return interpretBinary((ExprBinary*)expr);
        case EXPR_UNARY  : return interpretUnary((ExprUnary*)expr);
        case EXPR_NUMBER : return interpretNumber((ExprNumber*)expr);
        case EXPR_VAR    : return interpretVar((ExprVar*)expr);
    }

    return 0;
}

void interpretStmt(Stmt* stmt);

void interpretSeq(StmtSeq* seq) {
    while (seq != NULL) {
        interpretStmt(seq->stmt);
        seq = seq->next;
    }
}

void interpretAssign(StmtAssign* stmt) {
    ExprVar* exprVar = (ExprVar*)stmt->left;
    ObjString* varName = exprVar->name;
    int value = interpret(stmt->right);
    tableSet(&interp.strings, varName, (Value){ VALUE_NUMBER, value });
}

void interpretPrint(StmtPrint* stmt) {
    printf("%d\n", interpret(stmt->expr));
}

void interpretIf(StmtIf* stmt) {
    if (interpret(stmt->condition)) {
        interpretSeq(stmt->thenBranch);
    } else {
        interpretStmt(stmt->elseBranch);
    }
}

void interpretWhile(StmtWhile* stmt) {
    while (interpret(stmt->condition)) {
        interpretSeq(stmt->body);
    }
}

void interpretStmt(Stmt* stmt) {
    if (stmt == NULL) return;

    switch (stmt->type) {
        case STMT_SEQUENCE: interpretSeq((StmtSeq*)stmt); break;
        case STMT_ASSIGN: interpretAssign((StmtAssign*)stmt); break;
        case STMT_PRINT: interpretPrint((StmtPrint*)stmt); break;
        case STMT_IF: interpretIf((StmtIf*)stmt); break;
        case STMT_WHILE: interpretWhile((StmtWhile*)stmt); break;
    }
}

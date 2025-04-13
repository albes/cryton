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

BigInt interpretBinary(ExprBinary* expr) {
    BigInt result = bigint_from_int(0);
    BigInt left = interpretExpr(expr->left);
    BigInt right = interpretExpr(expr->right);
    BigInt zero = bigint_from_int(0);
    
    switch (expr->operator) {
        case TOKEN_PLUS        : bigint_add(&result, &left, &right); break;// interpretExpr(expr->left) +  interpretExpr(expr->right);
        case TOKEN_MINUS       : bigint_sub(&result, &left, &right); break;// interpretExpr(expr->left) -  interpretExpr(expr->right);
        case TOKEN_LESS        : bigint_init(&result, bigint_abs_compare(&left, &right) < 0); break;// interpretExpr(expr->left) <  interpretExpr(expr->right);
        case TOKEN_GREATER     : bigint_init(&result, bigint_abs_compare(&left, &right) > 0); break;//interpretExpr(expr->left) >  interpretExpr(expr->right);
        case TOKEN_EQUAL_EQUAL : bigint_init(&result, bigint_abs_compare(&left, &right) == 0); break;
        case TOKEN_BANG_EQUAL  : bigint_init(&result, bigint_abs_compare(&left, &right) != 0); break;
        case TOKEN_AND         : bigint_init(&result, bigint_abs_compare(&left, &zero) != 0
                                                   && bigint_abs_compare(&right, &zero) != 0);
                                 break;
        case TOKEN_OR          : bigint_init(&result, bigint_abs_compare(&left, &zero) != 0
                                                   || bigint_abs_compare(&right, &zero) != 0);
                                 break;
    }

    return result;
}

BigInt interpretUnary(ExprUnary* expr) {
    BigInt result = bigint_from_int(0);
    BigInt zero = bigint_from_int(0);

    BigInt val = interpretExpr(expr->right);

    switch (expr->operator) {
        case TOKEN_MINUS : 
            val.sign = -val.sign;
            return val;
        case TOKEN_NOT   :
            if (bigint_abs_compare(&val, &zero) == 0){
                bigint_init(&val, 1);
            }
            else{
                bigint_init(&val, 0);
            }
            return val;
    }

    return result;
}

BigInt interpretNumber(ExprNumber* expr) {
    return expr->value;
}

// TODO: report an error if variable is uninitialized
BigInt interpretVar(ExprVar* expr) {
    Value value = { VALUE_NUMBER, bigint_from_int(0) };
    tableGet(&interp.strings, expr->name, &value);
    return value.number;
}

BigInt interpretExpr(Expr* expr) {
    switch (expr->type) {
        case EXPR_BINARY : return interpretBinary((ExprBinary*)expr);
        case EXPR_UNARY  : return interpretUnary((ExprUnary*)expr);
        case EXPR_NUMBER : return interpretNumber((ExprNumber*)expr);
        case EXPR_VAR    : return interpretVar((ExprVar*)expr);
    }

    return bigint_from_int(0);
}

void interpretStmt(Stmt* stmt);

void interpretAssign(StmtAssign* stmt) {
    ExprVar* exprVar = (ExprVar*)stmt->left;
    ObjString* varName = exprVar->name;
    BigInt number = interpretExpr(stmt->right);
    tableSet(&interp.strings, varName, (Value){ VALUE_NUMBER, number });
}

void interpretPrint(StmtPrint* stmt) {
    BigInt num = interpretExpr(stmt->expr);
    bigint_print(&num);
    putchar('\n');
}

void interpretIf(StmtIf* stmt) {
    BigInt val = interpretExpr(stmt->condition);
    BigInt zero = bigint_from_int(0);

    if (bigint_abs_compare(&val, &zero) != 0) {
        interpret(stmt->thenBranch);
    } else {
        interpret(stmt->elseBranch);
    }
}

void interpretWhile(StmtWhile* stmt) {
    BigInt val = interpretExpr(stmt->condition);
    BigInt zero = bigint_from_int(0);

    while (bigint_abs_compare(&val, &zero) != 0) {
        interpret(stmt->body);
        val = interpretExpr(stmt->condition);
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

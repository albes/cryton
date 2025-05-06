#include <stdio.h>

#include "common.h"
#include "object.h"
#include "table.h"
#include "interpreter.h"

Interp interp;

void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    longjmp(interp.errJmpBuf, 1);
}

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
        case TOKEN_PLUS        : bigint_add(&result, &left, &right); break;
        case TOKEN_MINUS       : bigint_sub(&result, &left, &right); break;
        case TOKEN_LESS        : bigint_init(&result, bigint_abs_compare(&left, &right) < 0); break;
        case TOKEN_GREATER     : bigint_init(&result, bigint_abs_compare(&left, &right) > 0); break;
        case TOKEN_EQUAL_EQUAL : bigint_init(&result, bigint_abs_compare(&left, &right) == 0); break;
        case TOKEN_BANG_EQUAL  : bigint_init(&result, bigint_abs_compare(&left, &right) != 0); break;
        case TOKEN_AND         : bigint_init(&result, bigint_abs_compare(&left, &zero) != 0
                                                   && bigint_abs_compare(&right, &zero) != 0); break;
        case TOKEN_OR          : bigint_init(&result, bigint_abs_compare(&left, &zero) != 0
                                                   || bigint_abs_compare(&right, &zero) != 0); break;
    }

    return result;
}

BigInt interpretUnary(ExprUnary* expr) {
    BigInt result = bigint_from_int(0);
    BigInt zero = bigint_from_int(0);
    BigInt val = interpretExpr(expr->right);

    switch (expr->operator) {
        case TOKEN_MINUS:
            val.sign = -val.sign;
            return val;
        case TOKEN_NOT:
            if (bigint_abs_compare(&val, &zero) == 0)
                bigint_init(&val, 1);
            else
                bigint_init(&val, 0);
            return val;
    }

    return result;
}

void saveCategory(RuntimeCategory* cat) {
    Value val = { .type = VALUE_CATEGORY, .category = cat };
    tableSet(&interp.strings, cat->name, val);
}

RuntimeCategory* getCategoryByName(ObjString* name) {
    Value val;
    if (tableGet(&interp.strings, name, &val) && val.type == VALUE_CATEGORY) {
        return val.category;
    }
    return NULL;
}

bool isObjectInCategory(RuntimeCategory* cat, BigInt* obj) {
    for (int i = 0; i < cat->objects.count; i++) {
        if (bigint_abs_compare(&cat->objects.values[i], obj) == 0) {
            return true;
        }
    }
    return false;
}

bool dfs(RuntimeCategory* cat, BigInt* current, BigInt* target, bool* visited, int objectCount) {
    for (int i = 0; i < cat->homset.count; i++) {
        Morphism* m = &cat->homset.morphisms[i];

        if (bigint_abs_compare(&m->from, current) != 0) continue;

        for (int j = 0; j < m->toCount; j++) {
            BigInt* neighbor = &m->to[j];

            if (bigint_abs_compare(neighbor, target) == 0) return true;

            int idx = -1;
            for (int k = 0; k < cat->objects.count; k++) {
                if (bigint_abs_compare(neighbor, &cat->objects.values[k]) == 0) {
                    idx = k;
                    break;
                }
            }
            if (idx >= 0 && !visited[idx]) {
                visited[idx] = true;
                if (dfs(cat, neighbor, target, visited, objectCount)) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool isMorphismInCategory(RuntimeCategory* cat, BigInt* from, BigInt* to) {
    int count = cat->objects.count;
    bool visited[count];
    for (int i = 0; i < count; i++) visited[i] = false;

    for (int i = 0; i < count; i++) {
        if (bigint_abs_compare(from, &cat->objects.values[i]) == 0) {
            visited[i] = true;
            break;
        }
    }

    return dfs(cat, from, to, visited, count);
}

BigInt interpretIn(ExprIn* expr) {
    if (expr->name->type != EXPR_VAR) {
        runtimeError("'in' expects a category name on the right.");
    }

    ObjString* name = ((ExprVar*)expr->name)->name;
    RuntimeCategory* cat = getCategoryByName(name);
    if (!cat) {
        runtimeError("Undefined category '%.*s'", name->length, name->chars);
    }

    if (expr->element->type == EXPR_MORPHISM) {
        ExprMorphism* morph = (ExprMorphism*)expr->element;
        BigInt from = interpretExpr(morph->from);
        BigInt to   = interpretExpr(morph->to);

        return bigint_from_int(isMorphismInCategory(cat, &from, &to));
    } else {
        BigInt obj = interpretExpr(expr->element);
        return bigint_from_int(isObjectInCategory(cat, &obj));
    }
}

void interpretCategory(StmtCat* cat) {
    RuntimeCategory* runtimeCat = malloc(sizeof(RuntimeCategory));
    runtimeCat->name = cat->name;

    runtimeCat->objects.count = cat->objects.count;
    runtimeCat->objects.values = malloc(sizeof(Value) * cat->objects.count);
    for (int i = 0; i < cat->objects.count; i++) {
        runtimeCat->objects.values[i] = cat->objects.values[i];
    }

    runtimeCat->homset.count = cat->homset.count;
    runtimeCat->homset.morphisms = malloc(sizeof(Morphism) * cat->homset.count);
    for (int i = 0; i < cat->homset.count; i++) {
        Morphism* dest = &runtimeCat->homset.morphisms[i];
        Morphism* src = &cat->homset.morphisms[i];

        dest->from = src->from;
        dest->toCount = src->toCount;
        dest->to = malloc(sizeof(BigInt) * src->toCount);
        for (int j = 0; j < src->toCount; j++) {
            dest->to[j] = src->to[j];
        }
    }

    saveCategory(runtimeCat);
}

BigInt interpretNumber(ExprNumber* expr) {
    return expr->value;
}

BigInt interpretVar(ExprVar* expr) {
    Value val;
    if (!tableGet(&interp.strings, expr->name, &val) || val.type == VALUE_NULL) {
        runtimeError("Undefined variable '%.*s'", expr->name->length, expr->name->chars);
    }

    if (val.type != VALUE_NUMBER) {
        runtimeError("'%.*s' is not a variable.", expr->name->length, expr->name->chars);
    }

    return val.number;
}

BigInt interpretExpr(Expr* expr) {
    switch (expr->type) {
        case EXPR_BINARY : return interpretBinary((ExprBinary*)expr);
        case EXPR_UNARY  : return interpretUnary((ExprUnary*)expr);
        case EXPR_NUMBER : return interpretNumber((ExprNumber*)expr);
        case EXPR_VAR    : return interpretVar((ExprVar*)expr);
        case EXPR_IN     : return interpretIn((ExprIn*)expr);
    }
    return bigint_from_int(0);
}

void interpretAssign(StmtAssign* stmt) {
    ExprVar* exprVar = (ExprVar*)stmt->left;
    ObjString* varName = exprVar->name;
    BigInt number = interpretExpr(stmt->right);
    Value val = { .type = VALUE_NUMBER, .number = number };
    tableSet(&interp.strings, varName, val);
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
        case STMT_PRINT : interpretPrint((StmtPrint*)stmt); break;
        case STMT_IF    : interpretIf((StmtIf*)stmt); break;
        case STMT_WHILE : interpretWhile((StmtWhile*)stmt); break;
        case STMT_CAT   : interpretCategory((StmtCat*)stmt); break;
    }
}

void interpret(Stmt* stmts) {
    if (setjmp(interp.errJmpBuf) == 0) {
        while (stmts != NULL) {
            interpretStmt(stmts);
            stmts = stmts->next;
        }
    } else {
        // Jumped here from runtimeError
        fprintf(stderr, "Runtime error occurred. Aborting interpretation.\n");
    }
}

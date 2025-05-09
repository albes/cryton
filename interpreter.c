#include <stdio.h>

#include "common.h"
#include "object.h"
#include "table.h"
#include "interpreter.h"

BigInt interpretExpr(Expr* expr);
void interpret(Stmt* stmts);

Interp interp;

void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    longjmp(interp.errJmpBuf, 1);
}

const char* typeName(ValueType type) {
    switch (type) {
        case VALUE_CAT_TEMPLATE:
            return "Category Template";
        case VALUE_CATEGORY:
            return "Category";
        case VALUE_NULL:
            return "Null";
        case VALUE_NUMBER:
            return "Number";
        default:
            return "Unknown";
    }
}

void initInterp() {
    initTable(&interp.strings);
}

void freeInterp() {
    freeTable(&interp.strings, true);
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
    } else {
        runtimeError("Expected a variable of type category after 'in', but got '%s'.", typeName(val.type));
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
        runtimeError("Expected a variable of type after 'in'.");
    }

    ObjString* name = ((ExprVar*)expr->name)->name;
    RuntimeCategory* cat = getCategoryByName(name);

    if (expr->element->type == EXPR_MORPHISM) {
        ExprMorphism* morph = (ExprMorphism*)expr->element;
        BigInt from = interpretExpr(morph->from);
        BigInt to   = interpretExpr(morph->to);

        return bigint_from_int(isMorphismInCategory(cat, &from, &to));
    } else {
        BigInt obj = interpretExpr(expr->element);
        return bigint_from_int(isObjectInCategory(cat, &obj));
    }

    // bigint_from_int(0);
}

void freeRuntimeCategory(RuntimeCategory* cat) {
    if (!cat) return;

    if (cat->objects.values) {
        free(cat->objects.values);
    }

    if (cat->homset.morphisms) {
        for (int i = 0; i < cat->homset.count; ++i) {
            if (cat->homset.morphisms[i].to) {
                free(cat->homset.morphisms[i].to);
            }
        }
        free(cat->homset.morphisms);
    }

    free(cat);
}

void interpretCategory(ExprCatInit* expr, ObjString* varName) {
    Value tmplVal;
    if (!tableGet(&interp.strings, expr->callee, &tmplVal) || tmplVal.type != VALUE_CAT_TEMPLATE) {
        runtimeError("Expected a variable of type 'Category Template', but got '%.*s' of type '%s'.",
                     expr->callee->length, expr->callee->chars, typeName(tmplVal.type));
    }

    CategoryTemplate* cat = tmplVal.template;
    if (expr->argCount != cat->paramCount) {
        runtimeError("Argument count does not match the parameters count of Category Template '%s' when creating Category '%s'.",
                        cat->name->chars, varName->chars);
    }

    Table globalStrings = interp.strings;

    Table templateArgs;
    initTable(&templateArgs);

    // Set up rollback
    jmp_buf originalBuf;
    memcpy(&originalBuf, &interp.errJmpBuf, sizeof(jmp_buf));

    RuntimeCategory* runtimeCat = NULL;

    if (setjmp(interp.errJmpBuf) != 0) {
        // An error occurred, free temp state
        interp.strings = globalStrings;
        freeTable(&templateArgs, false);
        freeRuntimeCategory(runtimeCat);  // safe even if NULL
        memcpy(&interp.errJmpBuf, &originalBuf, sizeof(jmp_buf));
        longjmp(interp.errJmpBuf, 1);
    }

    // Interpret arguments
    for (int i = 0; i < expr->argCount; ++i) {
        BigInt value = interpretExpr(expr->args[i]);
        tableSet(&templateArgs, cat->params[i], (Value){ .type = VALUE_NUMBER, .number = value });
    }

    interp.strings = templateArgs;

    runtimeCat = malloc(sizeof(RuntimeCategory));
    runtimeCat->homset.morphisms = NULL;
    runtimeCat->name = varName;

    // Objects
    runtimeCat->objects.count = cat->objects.count;
    runtimeCat->objects.values = malloc(sizeof(BigInt) * cat->objects.count);
    for (int i = 0; i < cat->objects.count; i++) {
        runtimeCat->objects.values[i] = interpretExpr(cat->objects.values[i]);
    }

    // Morphisms
    runtimeCat->homset.count = 0;
    runtimeCat->homset.morphisms = malloc(sizeof(Morphism) * cat->homset.count);
    for (int i = 0; i < cat->homset.count; i++) {
        TmplAdjMorphisms* src = &cat->homset.morphisms[i]; // Template morphisms of type Expr*
        Morphism* dest = &runtimeCat->homset.morphisms[i];
    
        dest->from = interpretExpr(src->from);
        dest->toCount = src->toCount;
        dest->to = malloc(sizeof(BigInt) * src->toCount);
    
        runtimeCat->homset.count++;
        for (int j = 0; j < src->toCount; j++) {
            dest->to[j] = interpretExpr(src->to[j]);
        }
    }

    // Done successfully
    interp.strings = globalStrings;
    freeTable(&templateArgs, false);
    saveCategory(runtimeCat);

    // Restore old jump buffer (important!)
    memcpy(&interp.errJmpBuf, &originalBuf, sizeof(jmp_buf));
}



void interpretCategoryTemplate(StmtCat* cat) {
    CategoryTemplate* templ = malloc(sizeof(CategoryTemplate));
    templ->name = cat->name;
    templ->params = cat->params;
    templ->paramCount = cat->paramCount;
    templ->objects = cat->objects;
    templ->homset = cat->homset;

    Value val = {
        .type = VALUE_CAT_TEMPLATE,
        .template = templ
    };

    tableSet(&interp.strings, templ->name, val);
}

BigInt interpretNumber(ExprNumber* expr) {
    return expr->value;
}

BigInt interpretVar(ExprVar* expr) {
    Value val;
    if (!tableGet(&interp.strings, expr->name, &val) || val.type == VALUE_NULL) {
        runtimeError("Undefined variable '%.*s'.", expr->name->length, expr->name->chars);
    }

    if (val.type != VALUE_NUMBER) {
        const char* typeStr;
    
        runtimeError("Cannot assign variable '%.*s' of type '%s'.",
                     expr->name->length, expr->name->chars, typeName(val.type));
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

    Value val;

    if (stmt->right->type == EXPR_CAT_INIT) {
        ExprCatInit* init = (ExprCatInit*)stmt->right;
        
        interpretCategory(init, varName);
    } else {
        BigInt number = interpretExpr(stmt->right);
        val.type = VALUE_NUMBER;
        val.number = number;
        tableSet(&interp.strings, varName, val);
    }
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
        case STMT_CAT   : interpretCategoryTemplate((StmtCat*)stmt); break;
    }
}

void interpret(Stmt* stmts) {
    while (stmts != NULL) {
        interpretStmt(stmts);
        stmts = stmts->next;
    }
}

void runInterp(Stmt* stmts) {
    if (setjmp(interp.errJmpBuf) == 0) {
        interpret(stmts);
    } else {
        // Jumped here from runtimeError
        fprintf(stderr, "Runtime error occurred. Aborting interpretation.\n");
    }
}

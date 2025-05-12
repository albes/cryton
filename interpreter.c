#include <stdio.h>

#include "common.h"
#include "object.h"
#include "table.h"
#include "interpreter.h"

Value interpretExpr(Expr* expr);
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

Value makeValue(ValueType type, BigInt number) {
    Value val;
    val.type = type;
    val.number = number;
    return val;
}

void initInterp() {
    initTable(&interp.strings);
}

void freeInterp() {
    freeTable(&interp.strings, true);
}

Value interpretBinary(ExprBinary* expr) {
    Value leftVal = interpretExpr(expr->left);
    Value rightVal = interpretExpr(expr->right);

    if (leftVal.type != VALUE_NUMBER || rightVal.type != VALUE_NUMBER) {
        runtimeError("Binary operators can only be applied to numbers.\n"
                        "But got values of types '%s' and '%s'",
                        typeName(leftVal.type), typeName(rightVal.type));
    }

    BigInt left = leftVal.number;
    BigInt right = rightVal.number;
    BigInt result = bigint_from_int(0);
    BigInt zero = bigint_from_int(0);

    switch (expr->operator) {
        case TOKEN_PLUS:
            bigint_add(&result, &left, &right);
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_MINUS:
            bigint_sub(&result, &left, &right);
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_LESS:
            bigint_init(&result, bigint_abs_compare(&left, &right) < 0);
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_GREATER:
            bigint_init(&result, bigint_abs_compare(&left, &right) > 0);
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_EQUAL_EQUAL:
            bigint_init(&result, bigint_abs_compare(&left, &right) == 0);
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_BANG_EQUAL:
            bigint_init(&result, bigint_abs_compare(&left, &right) != 0);
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_AND:
            bigint_init(&result,
                bigint_abs_compare(&left, &zero) != 0 &&
                bigint_abs_compare(&right, &zero) != 0);
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_OR:
            bigint_init(&result,
                bigint_abs_compare(&left, &zero) != 0 ||
                bigint_abs_compare(&right, &zero) != 0);
            return makeValue(VALUE_NUMBER, result);
    }

    return makeValue(VALUE_NULL, bigint_from_int(0)); // fallback for unknown operator
}


Value interpretUnary(ExprUnary* expr) {
    Value val = interpretExpr(expr->right);

    if (val.type != VALUE_NUMBER) {
        runtimeError("Unary operator can only be applied to numbers.\n"
                        "But got value of type '%s'",
                        typeName(val.type));
    }

    BigInt result = val.number;
    BigInt zero = bigint_from_int(0);

    switch (expr->operator) {
        case TOKEN_MINUS:
            result.sign = -result.sign;
            return makeValue(VALUE_NUMBER, result);

        case TOKEN_NOT:
            if (bigint_abs_compare(&result, &zero) == 0)
                result = bigint_from_int(1);
            else
                result = bigint_from_int(0);
            return makeValue(VALUE_NUMBER, result);
    }

    return makeValue(VALUE_NULL, bigint_from_int(0));
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

bool isObjectInCategory(RuntimeCategory* cat, Value* obj) {
    for (int i = 0; i < cat->objects.count; i++) {
        Value* current = &cat->objects.values[i];

        if (obj->type != current->type) continue;

        if (obj->type == VALUE_NUMBER) {
            if (bigint_abs_compare(&current->number, &obj->number) == 0) return true;
        } else if (obj->type == VALUE_CATEGORY) {
            if (current->category == obj->category) return true;
        }
    }
    return false;
}


bool dfs(RuntimeCategory* cat, Value* current, Value* target, bool* visited, int objectCount) {
    for (int i = 0; i < cat->homset.count; i++) {
        Morphism* m = &cat->homset.morphisms[i];

        if (m->from.type != current->type) continue;

        bool fromMatch = false;
        if (current->type == VALUE_NUMBER) {
            fromMatch = bigint_abs_compare(&m->from.number, &current->number) == 0;
        } else if (current->type == VALUE_CATEGORY) {
            fromMatch = m->from.category == current->category;
        }

        if (!fromMatch) continue;

        for (int j = 0; j < m->toCount; j++) {
            Value* neighbor = &m->to[j];

            bool typeMatch = false;
            if (neighbor->type == target->type) typeMatch = true;

            bool targetMatch = false;
            if (typeMatch) {
                if (target->type == VALUE_NUMBER) {
                    targetMatch = bigint_abs_compare(&neighbor->number, &target->number) == 0;
                } else if (target->type == VALUE_CATEGORY) {
                    targetMatch = neighbor->category == target->category;
                }
            }

            if (targetMatch) return true;

            // Check index for further traversal
            int idx = -1;
            for (int k = 0; k < cat->objects.count; k++) {
                Value* check = &cat->objects.values[k];
                if (check->type != neighbor->type) continue;

                if ((check->type == VALUE_NUMBER &&
                     bigint_abs_compare(&check->number, &neighbor->number) == 0) ||
                    (check->type == VALUE_CATEGORY &&
                     check->category == neighbor->category)) {
                    idx = k;
                    break;
                }
            }

            if (idx >= 0 && !visited[idx]) {
                visited[idx] = true;
                if (dfs(cat, neighbor, target, visited, objectCount)) return true;
            }
        }
    }

    return false;
}

bool isMorphismInCategory(RuntimeCategory* cat, Value* from, Value* to) {
    int count = cat->objects.count;
    bool visited[count];
    for (int i = 0; i < count; i++) visited[i] = false;

    for (int i = 0; i < count; i++) {
        Value* val = &cat->objects.values[i];
        if (val->type != from->type) continue;

        if ((val->type == VALUE_NUMBER &&
             bigint_abs_compare(&val->number, &from->number) == 0) ||
            (val->type == VALUE_CATEGORY &&
             val->category == from->category)) {
            visited[i] = true;
            break;
        }
    }

    return dfs(cat, from, to, visited, count);
}

Value interpretIn(ExprIn* expr) {
    if (expr->name->type != EXPR_VAR) {
        runtimeError("Expected a variable of type after 'in'.");
    }

    ObjString* name = ((ExprVar*)expr->name)->name;
    RuntimeCategory* cat = getCategoryByName(name);

    if (expr->element->type == EXPR_MORPHISM) {
        ExprMorphism* morph = (ExprMorphism*)expr->element;
        Value fromVal = interpretExpr(morph->from);
        Value toVal   = interpretExpr(morph->to);

        bool result = isMorphismInCategory(cat, &fromVal, &toVal);
        return makeValue(VALUE_NUMBER, bigint_from_int(result));
    } else {
        Value objVal = interpretExpr(expr->element);

        bool result = isObjectInCategory(cat, &objVal);
        return makeValue(VALUE_NUMBER, bigint_from_int(result));
    }
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

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) {
        return false;
    }

    switch (a.type) {
        case VALUE_NUMBER:
            return (bigint_abs_compare(&a.number, &b.number) == 0);
        default:
            // compare pointers
            return a.category == b.category;
    }
}

bool listContainsValue(ObjectList list, Value value) {
    for (int i = 0; i < list.count; ++i) {
        if (valuesEqual(list.values[i], value)) {
            return true;
        }
    }

    return false;
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
        Value value = interpretExpr(expr->args[i]);

        switch (value.type) {
            case VALUE_CAT_TEMPLATE:    runtimeError("Cannot pass variable '%s' of type '%s' to Category Template '%s'.",
                                            value.template->name->chars, typeName(value.type), tmplVal.template->name->chars);
            // case VALUE_CATEGORY:        runtimeError("Cannot pass variable '%s' of type '%s' to Category Template '%s'.",
            //                                 value.category->name->chars, typeName(value.type), tmplVal.template->name->chars);
        }

        tableSet(&templateArgs, cat->params[i], value);
    }

    interp.strings = templateArgs;

    runtimeCat = malloc(sizeof(RuntimeCategory));
    runtimeCat->objects.values = NULL;
    runtimeCat->homset.morphisms = NULL;
    runtimeCat->name = varName;

    // Objects
    runtimeCat->objects.count = cat->objects.count;
    runtimeCat->objects.values = malloc(sizeof(Value) * cat->objects.count);
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

        if (!listContainsValue(runtimeCat->objects, dest->from)) {
            runtimeError("Undeclared object inside morphism.");
        }

        dest->toCount = src->toCount;
        dest->to = malloc(sizeof(Value) * src->toCount);
    
        runtimeCat->homset.count++;
        for (int j = 0; j < src->toCount; j++) {
            dest->to[j] = interpretExpr(src->to[j]);

            if (!listContainsValue(runtimeCat->objects, dest->to[j])) {
                runtimeError("Undeclared object inside morphism.");
            }
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

Value interpretNumber(ExprNumber* expr) {
    return makeValue(VALUE_NUMBER, expr->value);
}

Value interpretVar(ExprVar* expr) {
    Value val;
    if (!tableGet(&interp.strings, expr->name, &val) || val.type == VALUE_NULL) {
        runtimeError("Undefined variable '%.*s'.", expr->name->length, expr->name->chars);
    }
    return val;
}

Value interpretExpr(Expr* expr) {
    switch (expr->type) {
        case EXPR_BINARY:  return interpretBinary((ExprBinary*)expr);
        case EXPR_UNARY:   return interpretUnary((ExprUnary*)expr);
        case EXPR_NUMBER:  return interpretNumber((ExprNumber*)expr);
        case EXPR_VAR:     return interpretVar((ExprVar*)expr);
        case EXPR_IN:      return interpretIn((ExprIn*)expr);
    }
    return makeValue(VALUE_NULL, bigint_from_int(0));
}

void interpretAssign(StmtAssign* stmt) {
    ExprVar* exprVar = (ExprVar*)stmt->left;
    ObjString* varName = exprVar->name;

    if (stmt->right->type == EXPR_CAT_INIT) {
        ExprCatInit* init = (ExprCatInit*)stmt->right;
        
        interpretCategory(init, varName);
    } else {
        Value val = interpretExpr(stmt->right);
        switch (val.type) {
            case VALUE_CAT_TEMPLATE:    runtimeError("Cannot assign variable '%s' of type '%s'.",
                                            val.template->name->chars, typeName(val.type));
            case VALUE_CATEGORY:        runtimeError("Cannot assign variable '%s' of type '%s'.",
                                            val.category->name->chars, typeName(val.type));
        }
        tableSet(&interp.strings, varName, val);
    }
}

void interpretPrint(StmtPrint* stmt) {
    Value val = interpretExpr(stmt->expr);
    switch (val.type) {
        case VALUE_CAT_TEMPLATE:    runtimeError("Cannot print variable '%s' of type '%s'.",
                                        val.template->name->chars, typeName(val.type));
        case VALUE_CATEGORY:        runtimeError("Cannot print variable '%s' of type '%s'.",
                                        val.category->name->chars, typeName(val.type));
    }
    bigint_print(&val.number);
    putchar('\n');
}

void interpretIf(StmtIf* stmt) {
    Value val = interpretExpr(stmt->condition);
    BigInt zero = bigint_from_int(0);

    if (bigint_abs_compare(&val.number, &zero) != 0) {
        interpret(stmt->thenBranch);
    } else {
        interpret(stmt->elseBranch);
    }
}

void interpretWhile(StmtWhile* stmt) {
    Value val = interpretExpr(stmt->condition);
    BigInt zero = bigint_from_int(0);

    while (bigint_abs_compare(&val.number, &zero) != 0) {
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

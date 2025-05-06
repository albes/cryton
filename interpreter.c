#include <stdio.h>

#include "common.h"
#include "object.h"
#include "table.h"
#include "interpreter.h"

Interp interp;
RuntimeCategory* savedCategories[256];
int savedCategoryCount = 0;

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

void saveCategory(RuntimeCategory* cat) {
    for (int i = 0; i < savedCategoryCount; i++) {
        if (savedCategories[i]->name->length == cat->name->length &&
            memcmp(savedCategories[i]->name->chars, cat->name->chars, cat->name->length) == 0) {
            // Found existing category with the same name; overwrite it
            savedCategories[i] = cat;
            return;
        }
    }

    // Not found, append to the list
    savedCategories[savedCategoryCount++] = cat;
}

RuntimeCategory* getCategoryByName(ObjString* name) {
    for (int i = 0; i < savedCategoryCount; i++) {
        if (savedCategories[i]->name->length == name->length &&
            memcmp(savedCategories[i]->name->chars, name->chars, name->length) == 0) {
            return savedCategories[i];
        }
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

        // Skip if this morphism doesn't start at the current node
        if (bigint_abs_compare(&m->from, current) != 0) continue;

        for (int j = 0; j < m->toCount; j++) {
            BigInt* neighbor = &m->to[j];

            if (bigint_abs_compare(neighbor, target) == 0) return true;

            // Find index of neighbor in objects
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

    // Mark the starting node as visited
    for (int i = 0; i < count; i++) {
        if (bigint_abs_compare(from, &cat->objects.values[i]) == 0) {
            visited[i] = true;
            break;
        }
    }

    return dfs(cat, from, to, visited, count);
}

//InterpretIn only for objects in category
// BigInt interpretIn(ExprIn* expr) {
//     BigInt obj = interpretExpr(expr->element);  // e.g., 5
//     BigInt catNameVal = interpretExpr((Expr*)expr->name);

//     // Validate category name is a variable (a string-like representation)
//     if (expr->name->type != EXPR_VAR) {
//         fprintf(stderr, "Right-hand side of 'in' must be a category name.\n");
//         return bigint_from_int(0);
//     }

//     ObjString* name = ((ExprVar*)expr->name)->name;
//     RuntimeCategory* cat = getCategoryByName(name);
//     if (cat == NULL) {
//         fprintf(stderr, "Undefined category '%.*s'\n", name->length, name->chars);
//         return bigint_from_int(0);
//     }

//     if (isObjectInCategory(cat, &obj)) {
//         return bigint_from_int(1);
//     } else {
//         return bigint_from_int(0);
//     }
// }

BigInt interpretIn(ExprIn* expr) {
    if (expr->name->type != EXPR_VAR) {
        fprintf(stderr, "'in' expects a category name on the right.\n");
        return bigint_from_int(0);
    }

    ObjString* name = ((ExprVar*)expr->name)->name;
    RuntimeCategory* cat = getCategoryByName(name);
    if (!cat) {
        fprintf(stderr, "Undefined category '%.*s'\n", name->length, name->chars);
        return bigint_from_int(0);
    }

    if (expr->element->type == EXPR_MORPHISM) {
        ExprMorphism* morph = (ExprMorphism*)expr->element;
        BigInt from = interpretExpr(morph->from);
        BigInt to   = interpretExpr(morph->to);

        if (isMorphismInCategory(cat, &from, &to)) {
            return bigint_from_int(1);
        } else {
            return bigint_from_int(0);
        }

    } else {
        BigInt obj = interpretExpr(expr->element);
        if (isObjectInCategory(cat, &obj)) {
            return bigint_from_int(1);
        } else {
            return bigint_from_int(0);
        }
    }
}

void interpretCategory(StmtCat* cat) {
    // printf("Category: %.*s\n", cat->name->length, cat->name->chars);

    // Print and build the category
    RuntimeCategory* runtimeCat = malloc(sizeof(RuntimeCategory));
    runtimeCat->name = cat->name;

    // Deep copy the object values
    runtimeCat->objects.count = cat->objects.count;
    runtimeCat->objects.values = malloc(sizeof(Value) * cat->objects.count);
    for (int i = 0; i < cat->objects.count; i++) {
        runtimeCat->objects.values[i] = cat->objects.values[i];  // Assuming Value is POD
    }

    // Deep copy the morphisms
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

    // Save to global memory or interpreter state
    saveCategory(runtimeCat);
    // savedCategories[savedCategoryCount++] = runtimeCat;
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
        case EXPR_IN     : return interpretIn((ExprIn*)expr);
        //case EXPR_MORPHISM  : return bigint_from_int(0);
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
        case STMT_CAT: interpretCategory((StmtCat*)stmt); break;
    }
}

void interpret(Stmt* stmts) {
    while (stmts != NULL) {
        interpretStmt(stmts);
        stmts = stmts->next;
    }
}

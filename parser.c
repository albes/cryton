#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"
#include "common.h"
#include "object.h"
#include "parser.h"
#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

Parser parser;

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.   
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static ExprBinary* makeExprBinary(TokenType operator, Expr* left, Expr* right) {
    ExprBinary* expr = malloc(sizeof(ExprBinary));
    expr->type = EXPR_BINARY;
    expr->operator = operator;
    expr->left = left;
    expr->right = right;
    return expr;
}

static ExprUnary* makeExprUnary(TokenType operator, Expr* right) {
    ExprUnary* expr = malloc(sizeof(ExprUnary));
    expr->type = EXPR_UNARY;
    expr->operator = operator;
    expr->right = right;
    return expr;
}

static ExprNumber* makeExprNumber(BigInt value) {
    ExprNumber* expr = malloc(sizeof(ExprNumber));
    expr->type = EXPR_NUMBER;
    expr->value = value;
    return expr;
}

static ExprVar* makeExprVar(const char* name, int length) {
    ExprVar* expr = malloc(sizeof(ExprVar));
    expr->type = EXPR_VAR;
    expr->name = copyString(name, length);
    return expr;
}

static bool match(TokenType type) {
    if (parser.current.type != type) return false;
    advance();
    return true;
}

static Expr* expression();

static Expr* atom() {
    if (match(TOKEN_NUMBER)) {
        BigInt a = bigint_from_str(parser.previous.start, parser.previous.length);
        return (Expr*)makeExprNumber(a);
    }
    if (match(TOKEN_IDENTIFIER))
        return (Expr*)makeExprVar(parser.previous.start, parser.previous.length);

    if (match(TOKEN_LEFT_PAREN)) {
        Expr* expr = expression();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    errorAtCurrent("Expect expression.");
    return NULL;
}

static Expr* term() {
    if (parser.current.type == TOKEN_MINUS) {
        advance();
        TokenType operator = parser.previous.type;
        return (Expr*)makeExprUnary(operator, term());
    }

    return atom();
}

static Expr* sum() {
    Expr* expr = term();

    while (parser.current.type == TOKEN_PLUS ||
           parser.current.type == TOKEN_MINUS) {
        advance();
        TokenType operator = parser.previous.type;
        expr = (Expr*)makeExprBinary(operator, expr, term());
    }

    return expr;
}

static bool matchComparison() {
    switch (parser.current.type) {
        case TOKEN_LESS:
        case TOKEN_GREATER:
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_BANG_EQUAL:
            advance();
            return true;
        default:
            return false;
    }
}

static Expr* comparison() {
    Expr* expr = sum();

    while (matchComparison()) {
        TokenType operator = parser.previous.type;
        expr = (Expr*)makeExprBinary(operator, expr, sum());
    }

    return expr;
}

ExprIn* makeInExpr(Expr* element, ExprVar* name) {
    ExprIn* expr = malloc(sizeof(ExprIn));
    expr->expr.type = EXPR_IN;
    expr->element = element;
    expr->name = name;
    return expr;
}

ExprMorphism* makeMorphismExpr(Expr* from, Expr* to) {
    ExprMorphism* expr = malloc(sizeof(ExprMorphism));
    expr->type = EXPR_MORPHISM;
    expr->from = from;
    expr->to = to;
    return expr;
}


Expr* membership() {
    Expr* left = comparison();

    if (match(TOKEN_ARROW)) {
        Expr* right = comparison();
        // if (!right) error("Expected expression after '->'.");

        Expr* morph = (Expr*)makeMorphismExpr(left, right);

        consume(TOKEN_IN, "Expected 'in' after morphism.");
        consume(TOKEN_IDENTIFIER, "Expected identifier after morphism 'in'");
        ExprVar* name = makeExprVar(parser.previous.start, parser.previous.length);

        return (Expr*)makeInExpr(morph, name);
    } else if (match(TOKEN_IN)) {
        consume(TOKEN_IDENTIFIER, "Expected identifier after object 'in'");

        ExprVar* name = makeExprVar(parser.previous.start, parser.previous.length);
        return (Expr*)makeInExpr(left, name);
    }

    return left;
}

static Expr* inversion() {
    if (parser.current.type == TOKEN_NOT) {
        advance();
        return (Expr*)makeExprUnary(TOKEN_NOT, inversion());
    }

    return membership();
}

static Expr* conjunction() {
    Expr* expr = inversion();

    while (parser.current.type == TOKEN_AND) {
        advance();
        expr = (Expr*)makeExprBinary(TOKEN_AND, expr, inversion());
    }

    return expr;
}

static Expr* disjunction() {
    Expr* expr = conjunction();

    while (parser.current.type == TOKEN_OR) {
        advance();
        expr = (Expr*)makeExprBinary(TOKEN_OR, expr, conjunction());
    }

    return expr;
}

static Expr* expression() {
    return disjunction();
}

StmtAssign* makeStmtAssign(ExprVar* variable, Expr* expr) {
    StmtAssign* assign = malloc(sizeof(StmtAssign));
    assign->stmt.type = STMT_ASSIGN;
    assign->stmt.next = NULL;
    assign->left = variable;
    assign->right = expr;
    return assign;
}

StmtPrint* makeStmtPrint(Expr* expr) {
    StmtPrint* print = malloc(sizeof(StmtPrint));
    print->stmt.type = STMT_PRINT;
    print->stmt.next = NULL;
    print->expr = expr;
    return print;
}

StmtIf* makeStmtIf(Expr* condition, Stmt* thenBranch, Stmt* elseBranch) {
    StmtIf* ifStmt = malloc(sizeof(StmtIf));
    ifStmt->stmt.type = STMT_IF;
    ifStmt->stmt.next = NULL;
    ifStmt->condition = condition;
    ifStmt->thenBranch = thenBranch;
    ifStmt->elseBranch = elseBranch;
    return ifStmt;
}

StmtWhile* makeStmtWhile(Expr* condition, Stmt* body) {
    StmtWhile* whileStmt = malloc(sizeof(StmtWhile));
    whileStmt->stmt.type = STMT_WHILE;
    whileStmt->stmt.next = NULL;
    whileStmt->condition = condition;
    whileStmt->body = body;
    return whileStmt;
}

static Stmt* statement();

Stmt* block() {
    consume(TOKEN_NEWLINE, "Expect NEWLINE before block.");
    consume(TOKEN_INDENT, "Expect INDENT before block.");
    Stmt* block = statement();
    Stmt* tail = block;

    while (parser.current.type != TOKEN_EOF &&
           parser.current.type != TOKEN_DEDENT) {
        tail->next = statement();

        if (tail->next != NULL)
            tail = tail->next;
    }

    consume(TOKEN_DEDENT, "Expect DEDENT after block.");
    return block;
}

Stmt* whileStmt() {
    Expr* condition = expression();
    consume(TOKEN_COLON, "Expect ':' after while.");
    Stmt* body = block();

    return (Stmt*)makeStmtWhile(condition, body);
}


Stmt* ifStmt() {
    Expr* condition = expression();
    consume(TOKEN_COLON, "Expect ':' after condition.");
    Stmt* thenBranch = block();
    Stmt* elseBranch = NULL;

    if (match(TOKEN_ELIF)) {
        elseBranch = ifStmt();
    } else if (match(TOKEN_ELSE)) {
        consume(TOKEN_COLON, "Expect ':' after else.");
        elseBranch = block();
    }

    return (Stmt*)makeStmtIf(condition, thenBranch, elseBranch);
}

Stmt* print() {
    Stmt* stmt = (Stmt*)makeStmtPrint(expression());
    consume(TOKEN_NEWLINE, "Expect NEWLINE after print.");

    return stmt;
}

static bool isTermStart(TokenType type) {
    return type == TOKEN_NUMBER ||
           type == TOKEN_IDENTIFIER ||
           type == TOKEN_LEFT_PAREN ||
           type == TOKEN_MINUS;
}

static ExprCatInit* makeExprCatInit(ObjString* callee, Expr** args, int argCount) {
    ExprCatInit* expr = malloc(sizeof(ExprCatInit));
    expr->expr.type = EXPR_CAT_INIT;
    expr->callee = callee;
    expr->args = args;
    expr->argCount = argCount;
    return expr;
}

static Expr* parseMaybeCatInit(Expr* expr) {
    if (expr->type != EXPR_VAR) return expr;

    if (!match(TOKEN_LEFT_PAREN)) return expr;

    int capacity = 8;
    int count = 0;
    Expr** args = malloc(sizeof(Expr*) * capacity);

    while (parser.current.type != TOKEN_RIGHT_PAREN && parser.current.type != TOKEN_EOF) {
        if (count >= capacity) {
            capacity *= 2;
            args = realloc(args, sizeof(Expr*) * capacity);
        }
        args[count++] = expression();
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after category init arguments.");

    ObjString* callee = ((ExprVar*)expr)->name;
    free(expr);

    return (Expr*)makeExprCatInit(callee, args, count);
}

Stmt* assignment() {
    ExprVar* variable = makeExprVar(parser.previous.start, parser.previous.length);
    consume(TOKEN_EQUAL, "Expect '=' after variable.");

    Expr* expr = expression();
    expr = parseMaybeCatInit(expr);

    consume(TOKEN_NEWLINE, "Expect NEWLINE after assignment.");
    return (Stmt*)makeStmtAssign(variable, expr);
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        switch (parser.current.type) {
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            // case TOKEN_IDENTIFIER: // produces too many false errors
                return;
        }

        advance();

        if (parser.previous.type == TOKEN_NEWLINE)
            return;
    }
}

static void parseObjectSequence(Expr*** list, int* outCount, bool allow_newlines) {
    int capacity = 8;
    int count = 0;
    Expr** values = malloc(sizeof(Expr*) * capacity);

    while (isTermStart(parser.current.type)) {
        if (count >= capacity) {
            capacity *= 2;
            values = realloc(values, sizeof(Expr*) * capacity);
        }

        values[count++] = term();

        if (allow_newlines && parser.current.type == TOKEN_NEWLINE) {
            advance();
        }
    }

    *outCount = count;
    *list = values;
}



TmplAdjMorphisms parseMorphism() {
    TmplAdjMorphisms morph;
    morph.from = term();

    consume(TOKEN_ARROW, "Expect '->' in morphism.");
    parseObjectSequence(&morph.to, &morph.toCount, false);

    consume(TOKEN_NEWLINE, "Expect NEWLINE after morphism.");
    return morph;
}

void parseHomset(TmplHomSet* homset) {
    int capacity = 8;
    homset->morphisms = malloc(sizeof(TmplAdjMorphisms) * capacity);
    homset->count = 0;

    consume(TOKEN_NEWLINE, "Expect NEWLINE after 'hom'.");

    if (match(TOKEN_INDENT)) {
        while (parser.current.type != TOKEN_DEDENT &&
               parser.current.type != TOKEN_EOF) {
            if (homset->count >= capacity) {
                capacity *= 2;
                homset->morphisms = realloc(homset->morphisms, sizeof(TmplAdjMorphisms) * capacity);
            }

            homset->morphisms[homset->count++] = parseMorphism();
        }
        consume(TOKEN_DEDENT, "Expect DEDENT after homset.");
    }
}

void parseObjects(TmplObjects* objects) {
    consume(TOKEN_NEWLINE, "Expect NEWLINE before object list.");
    consume(TOKEN_INDENT, "Expect INDENT before object list.");

    parseObjectSequence(&objects->values, &objects->count, true);
    consume(TOKEN_DEDENT, "Expect DEDENT after object list.");
}

void parseCategoryBlock(TmplObjects* objects, TmplHomSet* homset) {
    consume(TOKEN_NEWLINE, "Expect NEWLINE before category block.");
    consume(TOKEN_INDENT, "Expect INDENT before category block.");

    consume(TOKEN_OBJ, "Expect 'obj' in category block.");
    consume(TOKEN_COLON, "Expect ':' after 'obj'.");

    parseObjects(objects);

    consume(TOKEN_HOM, "Expect 'hom' in category block.");
    consume(TOKEN_COLON, "Expect ':' after 'hom'.");

    parseHomset(homset);

    consume(TOKEN_DEDENT, "Expect DEDENT after category block.");
}

StmtCat* makeStmtCat(ObjString* name, ObjString** params, int paramCount, TmplObjects objects, TmplHomSet homset) {
    StmtCat* stmt = malloc(sizeof(StmtCat));
    stmt->stmt.type = STMT_CAT;
    stmt->stmt.next = NULL;
    stmt->name = name;
    stmt->params = params;
    stmt->paramCount = paramCount;
    stmt->objects = objects;
    stmt->homset = homset;
    return stmt;
}

Stmt* catStmt() {
    consume(TOKEN_IDENTIFIER, "Expect category name after 'cat'.");
    ObjString* name = copyString(parser.previous.start, parser.previous.length);

    consume(TOKEN_LEFT_PAREN, "Expect '(' after category name.");

    ObjString** params = NULL;
    int paramCount = 0;
    int capacity = 0;
    
    if (parser.current.type != (TOKEN_RIGHT_PAREN)) {
        capacity = 4;
        params = malloc(sizeof(ObjString*) * capacity);

        do {
            consume(TOKEN_IDENTIFIER, "Expect parameter name.");
            if (paramCount >= capacity) {
                capacity *= 2;
                params = realloc(params, sizeof(ObjString*) * capacity);
            }

            params[paramCount++] = copyString(parser.previous.start, parser.previous.length);
        } while (parser.current.type != TOKEN_RIGHT_PAREN);
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_COLON, "Expect ':' after category name.");

    TmplObjects objects;
    TmplHomSet homset;
    parseCategoryBlock(&objects, &homset);

    return (Stmt*)makeStmtCat(name, params, paramCount, objects, homset);
}


static Stmt* statement() {
    if (parser.panicMode)
        synchronize();

    if (match(TOKEN_IDENTIFIER)) return assignment();
    if (match(TOKEN_PRINT))      return      print();
    if (match(TOKEN_IF))         return     ifStmt();
    if (match(TOKEN_WHILE))      return  whileStmt();
    if (match(TOKEN_CAT))        return    catStmt();

    errorAtCurrent("Expect statement.");
    synchronize();
    return NULL;
}

bool parse(const char* source, Stmt** stmts) {
    initScanner(source);
    parser.hadError = false;
    parser.panicMode = false;
    advance();

    *stmts = NULL;

    while (parser.current.type != TOKEN_EOF) {
        *stmts = statement();

        if (*stmts != NULL)
            stmts = &((*stmts)->next); // sorry
    }

    return !parser.hadError;
}

static void freeExpr(Expr* expr);

static void freeExprBinary(ExprBinary* expr) {
    freeExpr(expr->left);
    freeExpr(expr->right);
    free(expr);
}

static void freeExprUnary(ExprUnary* expr) {
    freeExpr(expr->right);
    free(expr);
}

static void freeExprIn(ExprIn* expr) {
    freeExpr(expr->element);
    if (expr->name != NULL)
        free(expr->name);  // expr->name is an ExprVar* with allocated name string
    free(expr);
}

static void freeExprMorphism(ExprMorphism* expr) {
    freeExpr(expr->from);
    freeExpr(expr->to);
    free(expr);
}

static void freeExprCatInit(ExprCatInit* expr) {
    for (int i = 0; i < expr->argCount; i++) {
        freeExpr(expr->args[i]);
    }
    free(expr->args);
    free(expr);
}

static void freeExpr(Expr* expr) {
    if (expr == NULL)
        return;
    
    switch (expr->type) {
        case EXPR_BINARY : freeExprBinary((ExprBinary*)expr); break;
        case EXPR_UNARY  : freeExprUnary((ExprUnary*)expr);   break;
        case EXPR_NUMBER : free(expr);                        break;
        case EXPR_VAR    : free(expr);                        break;
        case EXPR_IN     : freeExprIn((ExprIn*)expr);         break;
        case EXPR_MORPHISM: freeExprMorphism((ExprMorphism*)expr); break;
        case EXPR_CAT_INIT: freeExprCatInit((ExprCatInit*)expr); break;


    }
}

static void freeStmtAssign(StmtAssign* stmt) {
    freeExpr((Expr*)stmt->left);
    freeExpr(stmt->right);
    free(stmt);
}

static void freeStmtPrint(StmtPrint* stmt) {
    freeExpr(stmt->expr);
    free(stmt);
}

static void freeStmtIf(StmtIf* stmt) {
    freeExpr(stmt->condition);
    freeAST(stmt->thenBranch);
    freeAST(stmt->elseBranch);
    free(stmt);
}
static void freeStmtWhile(StmtWhile* stmt) {
    freeExpr(stmt->condition);
    freeAST(stmt->body);
    free(stmt);
}

static void freeStmtCat(StmtCat* stmt) {
    for (int i = 0; i < stmt->objects.count; i++) {
        freeExpr(stmt->objects.values[i]);
    }
    free(stmt->objects.values);

    for (int i = 0; i < stmt->homset.count; i++) {
        freeExpr(stmt->homset.morphisms[i].from);
        for (int j = 0; j < stmt->homset.morphisms[i].toCount; j++) {
            freeExpr(stmt->homset.morphisms[i].to[j]);
        }
        free(stmt->homset.morphisms[i].to);
    }

    free(stmt->homset.morphisms);
    free(stmt->params);

    free(stmt);
}



void freeAST(Stmt* stmts) {
    if (stmts == NULL)
        return;

    while (stmts != NULL) {
        Stmt* next = stmts->next;
        switch (stmts->type) {
            case STMT_ASSIGN : freeStmtAssign((StmtAssign*)stmts); break;
            case STMT_PRINT  : freeStmtPrint((StmtPrint*)stmts);   break;
            case STMT_IF     : freeStmtIf((StmtIf*)stmts);         break;
            case STMT_WHILE  : freeStmtWhile((StmtWhile*)stmts);   break;
            case STMT_CAT    : freeStmtCat((StmtCat*)stmts);       break; //TODO repl problem
        }
        stmts = next;
    }
}

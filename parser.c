#include <stdio.h>
#include <stdlib.h>

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

static ExprNumber* makeExprNumber(int value) {
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
    if (match(TOKEN_NUMBER))
        return (Expr*)makeExprNumber(atoi(parser.previous.start));
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

static Expr* inversion() {
    if (parser.current.type == TOKEN_NOT) {
        advance();
        return (Expr*)makeExprUnary(TOKEN_NOT, inversion());
    }

    return comparison();
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

// TODO: add expression statements
Stmt* assignment() {
    ExprVar* variable = makeExprVar(parser.previous.start, parser.previous.length);
    consume(TOKEN_EQUAL, "Expect '=' after variable.");
    Stmt* stmt = (Stmt*)makeStmtAssign(variable, expression());
    consume(TOKEN_NEWLINE, "Expect NEWLINE after assignment.");

    return stmt;
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

static Stmt* statement() {
    if (parser.panicMode)
        synchronize();

    if (match(TOKEN_IDENTIFIER)) return assignment();
    if (match(TOKEN_PRINT))      return      print();
    if (match(TOKEN_IF))         return     ifStmt();
    if (match(TOKEN_WHILE))      return  whileStmt();

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

static void freeExpr(Expr* expr) {
    if (expr == NULL)
        return;
    
    switch (expr->type) {
        case EXPR_BINARY : freeExprBinary((ExprBinary*)expr); break;
        case EXPR_UNARY  : freeExprUnary((ExprUnary*)expr);   break;
        case EXPR_NUMBER : free(expr);                        break;
        case EXPR_VAR    : free(expr);                        break;
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
        }
        stmts = next;
    }
}

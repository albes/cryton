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

StmtSeq* makeStmtSeq(Stmt* stmt) {
    StmtSeq* seq = malloc(sizeof(StmtSeq));
    seq->type = STMT_SEQUENCE;
    seq->stmt = stmt;
    seq->next = NULL;
    return seq;
}

StmtAssign* makeStmtAssign(ExprVar* variable, Expr* expr) {
    StmtAssign* stmt = malloc(sizeof(StmtAssign));
    stmt->type = STMT_ASSIGN;
    stmt->left = variable;
    stmt->right = expr;
    return stmt;
}

StmtPrint* makeStmtPrint(Expr* expr) {
    StmtPrint* stmt = malloc(sizeof(StmtPrint));
    stmt->type = STMT_PRINT;
    stmt->expr = expr;
    return stmt;
}

StmtIf* makeStmtIf(Expr* condition, StmtSeq* thenBranch, Stmt* elseBranch) {
    StmtIf* stmt = malloc(sizeof(StmtIf));
    stmt->type = STMT_IF;
    stmt->condition = condition;
    stmt->thenBranch = thenBranch;
    stmt->elseBranch = elseBranch;
    return stmt;
}

StmtWhile* makeStmtWhile(Expr* condition, StmtSeq* body) {
    StmtWhile* stmt = malloc(sizeof(StmtWhile));
    stmt->type = STMT_WHILE;
    stmt->condition = condition;
    stmt->body = body;
    return stmt;
}

static Stmt* statement();

StmtSeq* block() {
    consume(TOKEN_NEWLINE, "Expect NEWLINE before block.");
    consume(TOKEN_INDENT, "Expect INDENT before block.");
    StmtSeq* seq = makeStmtSeq(statement());
    StmtSeq* tail = seq;

    while (parser.current.type != TOKEN_EOF &&
           parser.current.type != TOKEN_DEDENT) {
        tail->next = makeStmtSeq(statement());
        tail = tail->next;
    }

    consume(TOKEN_DEDENT, "Expect DEDENT after block.");
    return seq;
}

Stmt* whileStmt() {
    Expr* condition = expression();
    consume(TOKEN_COLON, "Expect ':' after while.");
    StmtSeq* body = block();

    return (Stmt*)makeStmtWhile(condition, body);
}

Stmt* ifStmt() {
    Expr* condition = expression();
    consume(TOKEN_COLON, "Expect ':' after condition.");
    StmtSeq* thenBranch = block();
    Stmt* elseBranch = NULL;

    if (match(TOKEN_ELIF)) {
        elseBranch = ifStmt();
    } else if (match(TOKEN_ELSE)) {
        consume(TOKEN_COLON, "Expect ':' after else.");
        elseBranch = (Stmt*)block();
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
            case TOKEN_IDENTIFIER:
                return;
        }

        // TODO: remove this line
        fprintf(stderr, "SKIPPING '%.*s'\n", parser.current.length, parser.current.start);
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
    return NULL;
}

bool parse(const char* source, Stmt** stmt) {
    initScanner(source);
    parser.hadError = false;
    parser.panicMode = false;
    advance();

    StmtSeq* tail = makeStmtSeq(statement());
    *stmt = (Stmt*)tail;

    while (parser.current.type != TOKEN_EOF) {
        tail->next = makeStmtSeq(statement());
        tail = tail->next;
    }

    return parser.hadError;
}

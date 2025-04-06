#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

const char* TokenName[] = {
    "TOKEN_LEFT_PAREN", "TOKEN_RIGHT_PAREN", "TOKEN_COLON",
    "TOKEN_PLUS", "TOKEN_MINUS", "TOKEN_SLASH", "TOKEN_STAR",
    "TOKEN_LESS", "TOKEN_GREATER",

    "TOKEN_EQUAL", "TOKEN_EQUAL_EQUAL",
    "TOKEN_BANG", "TOKEN_BANG_EQUAL",

    "TOKEN_AND", "TOKEN_OR", "TOKEN_NOT",
    "TOKEN_IF", "TOKEN_ELIF", "TOKEN_ELSE", "TOKEN_WHILE",
    "TOKEN_PRINT",

    "TOKEN_IDENTIFIER", "TOKEN_NUMBER",

    "TOKEN_NEWLINE", "TOKEN_INDENT", "TOKEN_DEDENT",

    "TOKEN_EOF", "TOKEN_ERROR",

    "TOKEN_CAT", "TOKEN_OBJ", "TOKEN_HOM", "TOKEN_ARROW"
};

typedef struct {
    const char* start;
    const char* current;
    int line;
    int indent;
    int indentLevel;
    int indentStack[128];
    bool blankLine;
} Scanner;

Scanner scanner;

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token makeTokenCustom(TokenType type, const char* lexeme) {
    Token token;
    token.type = type;
    token.start = lexeme;
    token.length = strlen(lexeme);
    token.line = scanner.line;

    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();

        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest,
                              TokenType type) {
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch (scanner.start[0]) {
        case 'a' : return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'e' :
            if (scanner.current - scanner.start > 3 && scanner.start[1] == 'l') {
                switch (scanner.start[2]) {
                    case 'i' : return checkKeyword(3, 1, "f", TOKEN_ELIF);
                    case 's' : return checkKeyword(3, 1, "e", TOKEN_ELSE);
                }
            }
        case 'i' : return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n' : return checkKeyword(1, 2, "ot", TOKEN_NOT);
        case 'o' : 
            if (scanner.current - scanner.start > 2 && scanner.start[1] == 'b')
                return checkKeyword(2, 1, "j", TOKEN_OBJ);
            return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p' : return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'w' : return checkKeyword(1, 4, "hile", TOKEN_WHILE);
        case 'c' : return checkKeyword(1, 2, "at", TOKEN_CAT);
        case 'h' : return checkKeyword(1, 2, "om", TOKEN_HOM);
    }

    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

static Token number() {
    while (isDigit(peek())) advance();
    return makeToken(TOKEN_NUMBER);
}

static bool pendingIndent() {
    return scanner.indent != scanner.indentStack[scanner.indentLevel] &&
           (!isWhitespace(peek()) || isAtEnd());
}

static void consumeIndent() {
    scanner.indent = 0;

    for (;;) {
        char c = peek();

        switch (c) {
            case ' ':
                scanner.indent++;
                advance();
                break;
            case '\t':
                scanner.indent += 8;
                advance();
                break;
            case '\r':
                advance();
                break;
            case '\n':
                advance();
                scanner.indent = 0;
                scanner.line++;
                break;
            case '#':
                while (peek() != '\n' && !isAtEnd()) advance();
                scanner.indent = 0;
                break;
            default:
                return;
        }
    }
}

static Token indent() {
    if (scanner.indent > scanner.indentStack[scanner.indentLevel]) {
        if (scanner.indentLevel == 128)
            errorToken("Reached indentation limit.");

        scanner.indentLevel++;
        scanner.indentStack[scanner.indentLevel] = scanner.indent;

        return makeTokenCustom(TOKEN_INDENT, "INDENT");
    }

    scanner.indentLevel--;

    if (scanner.indent <= scanner.indentStack[scanner.indentLevel]) {
        return makeTokenCustom(TOKEN_DEDENT, "DEDENT");
    }

    return errorToken("Invalid indent.");
}

static Token newline() {
    Token token = makeTokenCustom(TOKEN_NEWLINE, "NEWLINE");
    scanner.line++;
    consumeIndent();
    return token;
}

Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;

    if (pendingIndent()) return indent();
    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(' : return makeToken(TOKEN_LEFT_PAREN);
        case ')' : return makeToken(TOKEN_RIGHT_PAREN);
        case ':' : return makeToken(TOKEN_COLON);
        case '+' : return makeToken(TOKEN_PLUS);
        case '-' : 
            return makeToken(match('>') ? TOKEN_ARROW : TOKEN_MINUS);
        case '/' : return makeToken(TOKEN_SLASH);
        case '*' : return makeToken(TOKEN_STAR);
        case '<' : return makeToken(TOKEN_LESS);
        case '>' : return makeToken(TOKEN_GREATER);

        case '!' :
            return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=' :
            return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);

        case '\n' : return newline();
    }

    return errorToken("Unexpected character.");
}

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
    scanner.indentLevel = 0;
    scanner.indentStack[scanner.indentLevel] = 0;
    scanner.blankLine = true;
    consumeIndent();
}

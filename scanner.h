#ifndef cryton_scanner_h
#define cryton_scanner_h

typedef enum {
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_COLON,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_LESS, TOKEN_GREATER, 

    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_BANG, TOKEN_BANG_EQUAL,

    TOKEN_AND, TOKEN_OR, TOKEN_NOT,
    TOKEN_IF, TOKEN_ELIF, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_PRINT,

    TOKEN_IDENTIFIER, TOKEN_NUMBER,

    TOKEN_NEWLINE, TOKEN_INDENT, TOKEN_DEDENT,

    TOKEN_EOF, TOKEN_ERROR,

    TOKEN_CAT, TOKEN_OBJ, TOKEN_HOM, TOKEN_ARROW,

    TOKEN_IN
} TokenType;

extern const char* TokenName[];

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

void initScanner(const char* source);
Token scanToken();

#endif

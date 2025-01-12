#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "common.h"
#include "object.h"
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    size_t fileSize = 0;

    fseek(file, -1L, SEEK_END);
    if (fgetc(file) != '\n')
        fileSize++;

    fileSize += ftell(file) + 1;

    char* buffer = (char*)malloc(fileSize);

    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    rewind(file);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    if (buffer == NULL) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[fileSize - 2] = '\n';
    buffer[fileSize - 1] = '\0';

    fclose(file);
    return buffer;
}

static void printExpr(Expr* expr) {
    if (expr == NULL) {
        printf("NULL\n");
    } else if (expr->type == EXPR_BINARY) {
        ExprBinary* e = (ExprBinary*)expr;

        printf("Binary %s\n", TokenName[e->operator]);

        printExpr(e->left);
        printExpr(e->right);
    } else if (expr->type == EXPR_UNARY) {
        ExprUnary* e = (ExprUnary*)expr;

        printf("Unary %s\n", TokenName[e->operator]);
        printExpr(e->right);
    } else if (expr->type == EXPR_NUMBER) {
        ExprNumber* e = (ExprNumber*)expr;

        printf("Number %d\n", e->value);
    } else if (expr->type == EXPR_VAR) {
        ExprVar* e = (ExprVar*)expr;

        printf("Var %s\n", e->name->chars);
    } else {
        printf("Unknown expr\n");
    }
}

static void printStmt(Stmt* stmt);

static void printStmtAssign(StmtAssign* stmt) {
    printf("Assign\n");
    printExpr((Expr*)stmt->left);
    printExpr(stmt->right);
}

static void printStmtPrint(StmtPrint* stmt) {
    printf("Print\n");
    printExpr(stmt->expr);
}

static void printStmtIf(StmtIf* stmt) {
    printf("If\n");
    printExpr(stmt->condition);
    printStmt(stmt->thenBranch);
    if (stmt->elseBranch) {
        printf("Else\n");
        printStmt(stmt->elseBranch);
    }
}

static void printStmtWhile(StmtWhile* stmt) {
    printf("While\n");
    printExpr(stmt->condition);
    printStmt(stmt->body);
}

static void printStmt(Stmt* stmt) {
    if (stmt == NULL) {
        printf("NULL\n");
        return;
    }

    printf("Begin body\n");
    while (stmt != NULL) {
        switch (stmt->type) {
            case STMT_ASSIGN   : printStmtAssign((StmtAssign*)stmt); break;
            case STMT_PRINT    : printStmtPrint((StmtPrint*)stmt);   break;
            case STMT_IF       : printStmtIf((StmtIf*)stmt);         break;
            case STMT_WHILE    : printStmtWhile((StmtWhile*)stmt);   break;
            default            : printf("Unknown stmt\n");           break;
        }
        stmt = stmt->next;
    }
    printf("End body\n");
}

static void runFile(const char* path) {
    char* source = readFile(path);

    initScanner(source);

    // int line = -1;

    // for (;;) {
    //     Token token = scanToken();

    //     if (token.line != line) {
    //         printf("%4d ", token.line);
    //         line = token.line;
    //     } else {
    //         printf("   | ");
    //     }

    //     printf("%s '%.*s'\n", TokenName[token.type], token.length, token.start);

    //     if (token.type == TOKEN_EOF) break;
    // }

    Stmt* stmts;

    if (parse(source, &stmts)) {
        interpret(stmts);
    }

    freeAST(stmts);
    free(source);
}

static void repl() {
    FILE* file = fopen("log.txt", "ab");

    char* line;
    int currentSize;

    for (;;) {
        line = readline(">>> ");  // Read input using readline
        if (!line) {  // Handle end-of-input (CTRL+D)
            printf("\n");
            break;
        }

        // Add newline at the end of the input to mimic fgets() behavior
        currentSize = strlen(line);
        if (currentSize < 4095) {
            line = realloc(line, currentSize + 2);  // Resize memory to hold the newline
            line[currentSize] = '\n';  // Add newline
            line[currentSize + 1] = '\0';  // Null-terminate
            currentSize++;
        }

        add_history(line);  // Add input to history

        // Check for whitespace-only input
        char* head = line;
        while (*head != '\0' && isspace((unsigned char)*head))
            ++head;

        if (*head != '\0') {
            Stmt* stmts;
            fwrite(line, sizeof(char), currentSize, file);

            int length = strlen(line);  // Get the length of the line
            char lengthStr[50];
            sprintf(lengthStr, "Length: %d\n", length);  // Convert to string with a label
            fwrite(lengthStr, sizeof(char), strlen(lengthStr), file);

            if (parse(line, &stmts)) {
                interpret(stmts);
            }
            freeAST(stmts);
        }

        free(line);  // Free the memory allocated by readline
    }
    
    fclose(file);
}

int main(int argc, char* argv[]) {
    initInterp();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: cryton [path]\n");
        exit(64);
    }

    freeInterp();
    return 0;
}

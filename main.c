#include <stdio.h>
#include <stdlib.h>

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

static void printStmtSeq(StmtSeq* stmt) {
    printf("Sequence\n");
    printStmt(stmt->stmt);
    if (stmt->next)
        printStmtSeq(stmt->next);
    else
        printf("End sequence\n");
}

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
    printStmtSeq(stmt->thenBranch);
    if (stmt->elseBranch) {
        printf("Else\n");
        printStmt(stmt->elseBranch);
    }
}

static void printStmtWhile(StmtWhile* stmt) {
    printf("While\n");
    printExpr(stmt->condition);
    printStmtSeq(stmt->body);
}

static void printStmt(Stmt* stmt) {
    if (stmt == NULL) {
        printf("NULL\n");
        return;
    }

    switch (stmt->type) {
        case STMT_SEQUENCE : printStmtSeq((StmtSeq*)stmt);       break;
        case STMT_ASSIGN   : printStmtAssign((StmtAssign*)stmt); break;
        case STMT_PRINT    : printStmtPrint((StmtPrint*)stmt);   break;
        case STMT_IF       : printStmtIf((StmtIf*)stmt);         break;
        case STMT_WHILE    : printStmtWhile((StmtWhile*)stmt);   break;
        default            : printf("Unknown stmt\n");           break;
    }
}

static void runFile(const char* path) {
    char* source = readFile(path);

    // initScanner(source);

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

    // **********************************************

    // Expr* expr;

    // if (!parse(source, &expr))
    //     printf("ANSWER: %d\n", interpret(expr));

    // printExpr(expr);

    // **********************************************

    // Stmt* stmt;
    // parse(source, &stmt);
    // printStmt(stmt);

    // **********************************************

    Stmt* stmt;

    if (!parse(source, &stmt))
        interpretStmt(stmt);
    
    // printf("TREE\n");
    // printStmt(stmt);

    free(source);
}

static void repl() {
    char line[1024];

    for (;;) {
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        // interpret(line);
    }
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32  // If compiling on Windows
    #define USE_FGETS  // Use fgets for input
#else  // If compiling on Linux/Mac
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

#include "bigint.h"
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

static void printTokens(char* source) {
    initScanner(source);

    int line = -1;

    for (;;) {
        Token token = scanToken();

        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }

        printf("%s '%.*s'\n", TokenName[token.type], token.length, token.start);

        if (token.type == TOKEN_EOF) break;
    }
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

        printf("Number ");
        bigint_print(&e->value);
        putchar('\n');
    } else if (expr->type == EXPR_VAR) {
        ExprVar* e = (ExprVar*)expr;

        printf("Var %s\n", e->name->chars);
    } else if (expr->type == EXPR_IN) {
        ExprIn* e = (ExprIn*)expr;

        printf("In Expression \n");
        printExpr(e->element);

        printf("In Var %s \n", e->name->name->chars);
    } else if (expr->type == EXPR_MORPHISM) {
        ExprMorphism* e = (ExprMorphism*)expr;

        printf("Morphism\n");
        printExpr(e->from);
        printf("  | \n ");
        printf(" V\n");
        printExpr(e->to);

    } else if (expr->type == EXPR_CAT_INIT) {
        ExprCatInit* e = (ExprCatInit*)expr;

        printf("Category Constructor Call:\n");
        printf("  %.*s(", e->callee->length, e->callee->chars);
        for (int i = 0; i < e->argCount; i++) {
            printExpr(e->args[i]);
            if (i < e->argCount - 1) printf(", ");
        }
        printf(")\n");
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

static void printStmtCat(StmtCat* stmt) {
    printf("Category %s\n", stmt->name->chars);

    printf("   Objects\n");

    for(int i = 0; i < (stmt->objects.count); ++i) {
        printf("      ");
        printExpr(stmt->objects.values[i]);
        putchar('\n');
    }
    

    printf("   Homset\n");
    for(int i = 0; i < (stmt->homset.count); ++i) {
        printf("      ");
        TmplAdjMorphisms* morphism = &stmt->homset.morphisms[i];
        printExpr(morphism->from);
        printf(" -> ");

        for(int j = 0; j < (morphism->toCount); ++j) {
            printExpr(morphism->to[j]);
            putchar(' ');
        }

        putchar('\n');
    }
    // printStmt(stmt->objects);
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
            case STMT_CAT      : printStmtCat((StmtCat*)stmt);       break;
            default            : printf("Unknown stmt\n");           break;
        }
        stmt = stmt->next;
    }
    printf("End body\n");
}

static void runFile(const char* path, bool debug) {
    char* source = readFile(path);
    Stmt* stmts;

    if (debug) {
        printTokens(source);
    }

    if (!parse(source, &stmts)) {
        fprintf(stderr, "Could not parse file \"%s\".\n", path);
        exit(74);
    }

    if (debug) {
        printStmt(stmts);
    } else {
        runInterp(stmts);
    }

    freeAST(stmts);
    free(source);
}

static void repl() {
#ifdef USE_FGETS
    char line[4096];
    char* head = line;
    int currentSize = 0;

    for (;;) {
        printf(head == line ? ">>> " : "... ");

        if (!fgets(head, sizeof(line) - currentSize, stdin)) {
            printf("\n");
            break;
        }

        currentSize += strlen(head);

        // Escape newline with backslash
        if (currentSize > 1 && line[currentSize - 2] == '\\') {
            line[currentSize - 2] = '\n';
            --currentSize;
            head = line + currentSize;
            continue;
        }

        // Check for whitespace
        head = line;
        while (*head != '\0' && isspace((unsigned char)*head))
            ++head;

        // Interpret if lines are not blank
        if (*head != '\0') {
            Stmt* stmts;
            if (parse(line, &stmts)) {
                runInterp(stmts);
            }
            freeAST(stmts);
        }

        // Reset line
        currentSize = 0;
        head = line;
    }
#else
    char* line = NULL;  // Current input line
    int currentSize = 0;  // Total size of the input being built

    for (;;) {
        // Show prompt
        char* input = readline(currentSize == 0 ? ">>> " : "... ");
        if (!input) {  // Handle CTRL+D (EOF)
            printf("\n");
            break;
        }

        int inputLength = strlen(input);

        // Trim trailing spaces after the backslash
        int i = inputLength - 1;
        while (i >= 0 && isspace((unsigned char)input[i])) i--;

        // Check if the last non-whitespace character is a backslash
        int isContinuation = (i >= 0 && input[i] == '\\');

        if (isContinuation) {  // If it's a continuation line
            input[i] = '\n';  // Replace the backslash with a newline
            inputLength = i + 1;  // Update length
        } else {
            add_history(input);  // Add completed input to history
        }

        // Allocate space to append the new input
        line = realloc(line, currentSize + inputLength + 2);
        memcpy(line + currentSize, input, inputLength);  // Append input to the line
        currentSize += inputLength;

        // Ensure there's a final newline for the parser
        if (line[currentSize - 1] != '\n') {
            line[currentSize] = '\n';  // Add newline
            currentSize++;
        }

        line[currentSize] = '\0';  // Null-terminate
        free(input);  // Free temporary input

        if (!isContinuation) {  // If it's not a continuation, interpret the input
            char* head = line;
            while (*head != '\0' && isspace((unsigned char)*head))
                ++head;

            if (*head != '\0') {  // If the line is not empty
                Stmt* stmts;

                if (parse(line, &stmts)) {
                    runInterp(stmts);
                }
                freeAST(stmts);
            }

            // Reset after interpreting
            currentSize = 0;
            free(line);
            line = NULL;
        }
    }
#endif
}

int main(int argc, char* argv[]) {
    initInterp();
    char *path = NULL;
    bool debug = false;

    for (int i = 1; i < argc; ++i) {
        switch (argv[i][0]) {
            case '-':
                if (strcmp(argv[i], "-d") == 0) {
                    debug = true;
                }
                break;
            default:
                path = argv[i];
                break;
        }
    }

    if (argc > 3 || (!path && argc > 1)) {
        fprintf(stderr, "Usage: cryton [[-d] <path>]\n");
        exit(64);
    }

    if (path) {
        runFile(path, debug);
    } else {
        repl();
    }

    freeInterp();
    return 0;
}

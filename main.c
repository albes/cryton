#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"

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

static void runFile(const char* path) {
    char* source = readFile(path);

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

        if (token.type != TOKEN_NEWLINE)
            printf("%s '%.*s'\n", TokenName[token.type], token.length, token.start);
        else
            printf("%s\n", TokenName[token.type]);

        if (token.type == TOKEN_EOF) break;
    }

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
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: cryton [path]\n");
        exit(64);
    }

    return 0;
}

#ifndef cryton_interpreter_h
#define cryton_interpreter_h

#include "bigint.h"
#include "table.h"
#include "parser.h"
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

typedef struct {
    Table strings;
    jmp_buf errJmpBuf;
} Interp;


extern Interp interp;

#define MAX_CATEGORIES 256

typedef struct {
    RuntimeCategory categories[MAX_CATEGORIES];
    int categoryCount;
} Runtime;

void initInterp();
void freeInterp();
void runInterp(Stmt* stmts);

#endif

#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "object.h"
#include "interpreter.h"

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = malloc(sizeof(ObjString));
    string->chars = chars;
    string->length = length;
    string->hash = hash;
    
    Value val;
    val.type = VALUE_NULL;
    bigint_init(&val.number, 0);
    tableSet(&interp.strings, string, val);
    return string;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; ++i) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);

    ObjString* interned = tableFindString(&interp.strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = malloc(sizeof(char) * length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

void freeString(ObjString* string) {
    if (string == NULL) return;
    if (string->chars != NULL) free(string->chars);
    free(string);
}

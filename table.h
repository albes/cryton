#ifndef cryton_table_h
#define cryton_table_h

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table, bool freeKeys);
ObjString* tableFindKey(Table* table, Value val);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableDelete(Table* table, ObjString* key);
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);

#endif

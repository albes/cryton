#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "table.h"

#define TABLE_MAX_LOAD 0.75

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeCategory(RuntimeCategory* cat) {
    if (cat == NULL) return;

    // Free the array of objects
    if (cat->objects.values != NULL) {
        free(cat->objects.values);
        cat->objects.values = NULL;
    }

    // Free each morphism's `to` array
    for (int i = 0; i < cat->homset.count; i++) {
        Morphism* m = &cat->homset.morphisms[i];
        if (m->to != NULL) {
            free(m->to);
            m->to = NULL;
        }
    }

    // Free the array of morphisms
    if (cat->homset.morphisms != NULL) {
        free(cat->homset.morphisms);
        cat->homset.morphisms = NULL;
    }

    // Finally, free the category structure itself
    free(cat);
}

void freeTemplate(CategoryTemplate* templ) {
    if (templ == NULL) return;
    
    free(templ);
}

// Free table and all contained strings
void freeTable(Table* table) {
    if (table == NULL || table->entries == NULL)
        return;

    for (int i = 0; i < table->capacity; ++i) {
    Entry* entry = &table->entries[i];
    if (entry->key != NULL) {
        freeString(entry->key);  // Clean key
        if (entry->value.type == VALUE_CATEGORY && entry->value.category != NULL) {
            freeCategory(entry->value.category);
        }
        else if (entry->value.type == VALUE_CAT_TEMPLATE && entry->value.template != NULL) {
            freeTemplate(entry->value.template);
        }
    }
}

    free(table->entries);
    initTable(table);
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;

    for (;;) {
        Entry* entry = &entries[index];

        if (entry->key == NULL) {
            if (entry->value.type == VALUE_NULL) {
                return (tombstone != NULL ? tombstone : entry);
            } else {
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table* table, int capacity) {
    Entry* entries = (Entry*)malloc(sizeof(Entry) * capacity);
    for (int i = 0; i < capacity; ++i) {
        entries[i].key = NULL;
        entries[i].value.type = VALUE_NULL;
        bigint_init(&entries[i].value.number, 0); 
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; ++i) {
        Entry* entry  = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    free(table->entries);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableGet(Table* table, ObjString* key, Value* value) {
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

bool tableSet(Table* table, ObjString* key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;

    if (!isNewKey) {
        if (entry->value.type == VALUE_CATEGORY && entry->value.category != NULL) {
            freeCategory(entry->value.category);
        } else if (entry->value.type == VALUE_CAT_TEMPLATE && entry->value.template != NULL) {
            freeTemplate(entry->value.template);
        }
    }
    
    
    if (isNewKey && entry->value.type == VALUE_NULL)
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table* table, ObjString* key) {
    if (table->capacity == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Tombstone
    entry->key = NULL;
    entry->value.type = VALUE_NUMBER;
    bigint_init(&entry->value.number, 1); 
    return true;
}

ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;

    for (;;) {
        Entry* entry = &table->entries[index];

        if (entry->key == NULL) {
            if (entry->value.type == VALUE_NULL)
                return NULL;
        } else if (entry->key->length == length &&
                   entry->key->hash == hash &&
                   memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}

#ifndef cryton_value_h
#define cryton_value_h

#include "bigint.h"

typedef struct ObjString ObjString;

typedef struct {
    BigInt* values;
    int count;
} ObjectList;

typedef struct {
    BigInt from;
    BigInt* to;
    int toCount;
} Morphism;

typedef struct {
    Morphism* morphisms;
    int count;
} HomSet;

typedef struct {
    ObjString* name;
    ObjectList objects;
    HomSet homset;
} RuntimeCategory;

typedef enum {
    VALUE_NUMBER,
    VALUE_CATEGORY,
    VALUE_NULL
} ValueType;

typedef struct {
    ValueType type;
    union {
        BigInt number;
        RuntimeCategory* category;
    };
} Value;

#endif

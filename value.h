#ifndef cryton_value_h
#define cryton_value_h

#include "bigint.h"

typedef struct ObjString ObjString;
typedef struct Expr Expr;
typedef struct RuntimeCategory RuntimeCategory;
typedef struct CategoryTemplate CategoryTemplate;

typedef enum {
    VALUE_NUMBER,
    VALUE_CATEGORY,
    VALUE_CAT_TEMPLATE,
    VALUE_NULL
} ValueType;

typedef struct {
    ValueType type;
    union {
        BigInt number;
        RuntimeCategory* category;
        CategoryTemplate* template;
    };
} Value;

typedef struct {
    Value* values;
    int count;
} ObjectList;

typedef struct {
    Value from;
    Value* to;
    int toCount;
} Morphism;

typedef struct {
    Morphism* morphisms;
    int count;
} HomSet;

struct RuntimeCategory {
    ObjString* name;
    ObjectList objects;
    HomSet homset;
};

typedef struct ExprObjects {
    Expr** values;
    int count;
} TmplObjects;

typedef struct {
    Expr* from;
    Expr** to;
    int toCount;
} TmplAdjMorphisms;

typedef struct ExprHomSet {
    TmplAdjMorphisms* morphisms;
    int count;
} TmplHomSet;

struct CategoryTemplate {
    ObjString* name;
    ObjString** params;
    int paramCount;
    TmplObjects objects;
    TmplHomSet homset;
};

bool valuesEqual(Value a, Value b);

#endif
#ifndef cryton_value_h
#define cryton_value_h

#include "bigint.h"

typedef struct ObjString ObjString;
typedef struct Expr Expr;

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

typedef struct {
    ObjString* name;
    TmplObjects objects;
    TmplHomSet homset;
} CategoryTemplate;

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

#endif

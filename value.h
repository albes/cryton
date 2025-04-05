#ifndef cryton_value_h
#define cryton_value_h

#include "bigint.h"  // Assuming a bigint library is available

typedef struct ObjString ObjString;

typedef enum {
    VALUE_NUMBER, VALUE_NULL
} ValueType;

typedef struct {
    ValueType type;
    BigInt number;
} Value;

#endif

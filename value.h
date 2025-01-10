#ifndef cryton_value_h
#define cryton_value_h

typedef struct ObjString ObjString;

typedef enum {
    VALUE_NUMBER, VALUE_NULL
} ValueType;

typedef struct {
    ValueType type;
    int number;
} Value;

#endif

#ifndef cryton_object_h
#define cryton_object_h

#include "common.h"
#include "value.h"

struct ObjString {
    char* chars;
    int length;
    uint32_t hash;
};

ObjString* copyString(const char* chars, int length);
void freeString(ObjString* string);

#endif

#ifndef BIGINT_H
#define BIGINT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BIGINT_MAX_DIGITS 1024


typedef struct {
    int length;  // Number of digits
    int sign;    // 1 for positive, -1 for negative
    char digits[BIGINT_MAX_DIGITS]; // Store digits in reverse order
} BigInt;


void bigint_init(BigInt* num, int value);
BigInt bigint_from_int(int value);
BigInt bigint_from_str(const char* str, int len);
void bigint_add(BigInt* result, BigInt* a, BigInt* b);
void bigint_sub(BigInt* result, BigInt* a, BigInt* b);
//returns 1 if a > b, -1 if a < b, 0 if equal
int bigint_abs_compare(BigInt* a, BigInt* b);
void bigint_print(BigInt* num);
char* bigint_to_str_buf(BigInt* num, char* buffer, int buffer_size);

#endif // BIGINT_H

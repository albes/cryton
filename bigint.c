#include "bigint.h"

// Reverse a string (helper function)
void reverse(char* str, int len) {
    for (int i = 0; i < len / 2; ++i) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

// Initialize BigInt from an integer
void bigint_init(BigInt* num, int value) {
    num->sign = (value < 0) ? -1 : 1;  
    if (value < 0) value = -value;  // Work with absolute value

    snprintf(num->digits, BIGINT_MAX_DIGITS, "%d", value);
    num->length = strlen(num->digits);

    // Reverse digits for easier arithmetic
    reverse(num->digits, num->length);
}

BigInt bigint_from_int(int value) {
    BigInt num;

    num.sign = (value < 0) ? -1 : 1;
    if (value < 0) value = -value;

    snprintf(num.digits, BIGINT_MAX_DIGITS, "%d", value);
    num.length = strlen(num.digits);

    // Reverse digits for easier arithmetic
    reverse(num.digits, num.length);

    return num;
}

BigInt bigint_from_str(const char* str, int len) {
    BigInt num;
    int start = 0;

    // Handle sign
    if (str[0] == '-') {
        num.sign = -1;
        start = 1;
    } else {
        num.sign = 1;
        if (str[0] == '+') start = 1;
    }

    // Skip leading zeros
    while (str[start] == '0' && start < len - 1) start++;

    num.length = len - start;

    // Copy digits in reverse order
    for (int i = 0; i < num.length; i++) {
        char c = str[len - 1 - i];
        if (!isdigit(c)) {
            num.length = 0;
            num.sign = 1;
            num.digits[0] = '0';
            num.digits[1] = '\0';
            return num;  // Optional: handle invalid input
        }
        num.digits[i] = c;
    }

    num.digits[num.length] = '\0';
    return num;
}


// Print BigInt properly
void bigint_print(BigInt* num) {
    if (num->sign == -1) putchar('-');
    for (int i = num->length - 1; i >= 0; --i) {
        putchar(num->digits[i]);
    }
    // putchar('\n');
}

// Compare absolute values of two BigInts (returns 1 if a > b, -1 if a < b, 0 if equal)
int bigint_abs_compare(BigInt* a, BigInt* b) {
    // First check signs
    if (a->sign > b->sign) return 1;
    if (a->sign < b->sign) return -1;

    // If both are same sign, compare absolute values
    int sign = a->sign;
    if (a->length > b->length) return 1 * sign;
    if (a->length < b->length) return -1 * sign;
    for (int i = a->length - 1; i >= 0; --i) {
        if (a->digits[i] > b->digits[i]) return 1 * sign;
        if (a->digits[i] < b->digits[i]) return -1 * sign;
    }
    return 0;
}

// Add two BigInts, handling sign properly
void bigint_add(BigInt* result, BigInt* a, BigInt* b) {
    if (a->sign == b->sign) {
        result->sign = a->sign;
        int carry = 0, i;
        result->length = (a->length > b->length ? a->length : b->length);
        
        for (i = 0; i < result->length || carry; ++i) {
            int sum = carry + (i < a->length ? a->digits[i] - '0' : 0) + (i < b->length ? b->digits[i] - '0' : 0);
            result->digits[i] = (sum % 10) + '0';
            carry = sum / 10;
        }
        result->length = i;
    } else {
        // Convert subtraction of absolute values
        if (a->sign == -1) {
            a->sign = 1;
            bigint_sub(result, b, a);
            a->sign = -1;
        } else {
            b->sign = 1;
            bigint_sub(result, a, b);
            b->sign = -1;
        }
    }
}

// Subtract two BigInts, handling sign properly
void bigint_sub(BigInt* result, BigInt* a, BigInt* b) {
    if (a->sign != b->sign) {
        b->sign *= -1;
        bigint_add(result, a, b);
        b->sign *= -1;
        return;
    }

    // Ensure we subtract the smaller from the larger (in absolute value)
    int cmp = bigint_abs_compare(a, b);
    if (cmp == 0) {
        result->length = 1;
        result->digits[0] = '0';
        result->sign = 1;
        return;
    }

    const BigInt* larger = (cmp > 0) ? a : b;
    const BigInt* smaller = (cmp > 0) ? b : a;
    result->sign = (cmp > 0) ? a->sign : -a->sign;

    int borrow = 0;
    result->length = larger->length;
    for (int i = 0; i < result->length; ++i) {
        int diff = (larger->digits[i] - '0') - borrow - (i < smaller->length ? smaller->digits[i] - '0' : 0);
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result->digits[i] = diff + '0';
    }

    // Remove leading zeros
    while (result->length > 1 && result->digits[result->length - 1] == '0') {
        result->length--;
    }
}
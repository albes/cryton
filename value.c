#include "stdbool.h"
#include "value.h"

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) {
        return false;
    }

    switch (a.type) {
        case VALUE_NUMBER:
            return (bigint_abs_compare(&a.number, &b.number) == 0);
        default:
            // compare pointers
            return a.category == b.category;
    }
}
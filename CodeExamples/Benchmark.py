n = 1000
i = 1
while (n > 0):
    i = i + 1
    divisor = 2
    isPrime = 1
    while (isPrime and divisor < i):
        dividend = i
        while (dividend > 0):
            dividend = dividend - divisor
        if (dividend == 0):
            isPrime = 0
        divisor = divisor + 1
    if (isPrime):
        n = n - 1
print(i)
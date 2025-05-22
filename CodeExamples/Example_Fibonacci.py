a = 0
b = 1
i = 0
n = 10

while (i < n):
    print(a)
    tmp = b
    b = a + b
    a = tmp
    i = i + 1
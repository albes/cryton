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


#Output:
# 0
# 1
# 1
# 2
# 3
# 5
# 8
# 13
# 21
# 34    
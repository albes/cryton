a = 4
b = -11
result = 0

i = b
if b < 0:
    i = 0 - b

while i > 0:
    result = result + a
    i = i - 1

if b < 0:
    result = 0 - result

print(result)
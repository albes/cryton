a = 1
b = 2
c = 3

if a == 1:
    d = 99

while(a < 100):
    a = a + 1
    
    if a == c:
        d = 999999
    if a < 7:
        c = c + c


print a
print c
print d
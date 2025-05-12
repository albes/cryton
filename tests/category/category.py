a = 1
b = 2
c = 3

cat p(a b c):
    obj:
        40 5 65 412345 7 a
        (-5789654123) (a+1) 58421 59 46 -3 -4 -5 (1 and 6)
        b c
    hom:
        4 -> 9 99 999 1 2 -3 -4 -5 6 7 8 9 7 8 9 5 4 12 6 4 2 4 8 6 64  8 64 89 5 56489 795 5525 1 2 3 4 5 6 7 8 9 41 21 65  16 285 91 156 165 15461 451 35261 561 561 23 01 234181
        65 -> 5 4
        -4 -> 4
        8 -> 65
        a -> (a + 1)
        a -> b
        b -> c


categ = p(a+1 b c)

a = 41234
b = 51231
c = 61234
gg = p(a b c)

if 65 -> 5 in categ:
    print(1)
else:
    print(0)

# EXPECT ERROR: Undeclared object 4 of type 'Number' inside morphism.

# EXPECT: 1

print(41234 -> 41235 in gg)
# EXPECT: 1


print(1 -> 5555 in categ)
# EXPECT: 0

print(-3 in categ)
# EXPECT: 1
print(-4 in categ)
# EXPECT: 1
print(-9 in categ)
# EXPECT: 0
print(-5789654123 in categ)
# EXPECT: 1

p = 1
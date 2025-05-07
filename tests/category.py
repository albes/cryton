a = 1
b = 2
c = 3

cat p:
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


if 65 -> 5 in p:
    print(1)
else:
    print(0)

# EXPECT: 1

print(1 -> 2 in p)
# EXPECT: 1


print(1 -> 5555 in p)
# EXPECT: 0

print(-3 in p)
# EXPECT: 1
print(-4 in p)
# EXPECT: 1
print(-9 in p)
# EXPECT: 0
print(-5789654123 in p)
# EXPECT: 1



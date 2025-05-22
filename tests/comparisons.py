# EXPECT: 0
print(5 < 3)
# EXPECT: 1
print(5 > 3)
# EXPECT: 1
print(not 5 < 3)
# EXPECT: 0
print(5 == 3)
# EXPECT: 1
print(5 != 3)
# EXPECT: 1
print(not 5 == 3)
# EXPECT: 1
print(5 + 1 == 6)

a = 13
b = 10

# EXPECT: 1
print(a > b)
# EXPECT: 1
print(a == b + 3)
# EXPECT: 1
print((not a) == (not b))
# EXPECT: 1
print((not a) == (not not 0))

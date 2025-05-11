a = 10
b = 20

# EXPECT: 20
print(a + a)
# EXPECT: 10
print(a + a - (b - a))
# EXPECT: 1
print(not not a + a - (b - a))
# EXPECT: 1
print(0 or a)
# EXPECT: 0
print(0 and a)
# EXPECT: 0
print(not (a and b))
# EXPECT: 1
print(not (a and b) or 1)

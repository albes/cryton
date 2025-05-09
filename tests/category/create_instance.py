a = 1
b = 2
c = 3

cat T:
    obj:
        a b c
    hom:
        a -> b
        b -> c

instance = T(10 20 30)

print(10 -> 20 in instance)
print(30 in instance)

# EXPECT: 0
# EXPECT: 0

a = 10
b = 20
c = 30
instance = T(a b c)

print(10 -> 20 in instance)
print(30 in instance)

# EXPECT: 1
# EXPECT: 1
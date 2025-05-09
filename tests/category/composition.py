cat T(a b c d):
    obj:
        a b c d
    hom:
        a -> b
        b -> c
        c -> d

instance = T(10 20 30 40)

print(10 -> 20 in instance)
print(20 -> 30 in instance)
print(30 -> 40 in instance)
# EXPECT: 1
# EXPECT: 1
# EXPECT: 1

# composition
print(10 -> 30 in instance)
print(10 -> 40 in instance)

print(20 -> 40 in instance)

# EXPECT: 1
# EXPECT: 1
# EXPECT: 1
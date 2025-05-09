cat X:
    obj:
        1 2 3
    hom:
        1 -> 2
        2 -> 3

c = X()
print(1 in c)
print(2 -> 3 in c)

# EXPECT: 1
# EXPECT: 1
cat X(a):
    obj:
        (1) -2 a (a-100)
    hom:
        1 -> (a-100) a
        a -> -2

a = 1000
a = X(a)

print(a)

# EXPECT ERROR: Cannot print variable 'a' of type 'Category'.
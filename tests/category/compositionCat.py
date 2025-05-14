cat X(b):
    obj:
        2 3 (1 in b) b
    hom:
        # (1 in b) -> 2
        2 -> 3
        b -> 3 (1 in b)
        (1 in b) -> b

cat dummy():
    obj:
        1
    hom:
        1 -> 1


b = dummy()

c = X(b)

# EXPECT ERROR: Undeclared object 'b' of type 'Category' inside morphism.



print(b in b)
# EXPECT: 0

print(c in b)
# EXPECT: 0

print(b in c)
# EXPECT: 1

print(b -> 2 in c)
# EXPECT: 0
cat X(b):
    obj:
        2 3 (1 in b) b
    hom:
        # (1 in b) -> 2
        2 -> 3
        b -> 3 1
        1 -> b

cat dummy():
    obj:
        1
    hom:
        1 -> 1


b = dummy()

c = X(b)


# EXPECT ERROR: Undeclared object 'b' of type 'Category' inside morphism.


print(1 in c)
# EXPECT: 1
print(2 -> 3 in c)
# EXPECT: 1

print(b in c)
# EXPECT: 1

print(1 -> 3 in c)
# EXPECT: 1
print(1 -> b in c)
# EXPECT: 1
print(b -> b in c)
# EXPECT: 1
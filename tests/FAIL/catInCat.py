cat X(b):
    obj:
        2 3 (1 in b)
    hom:
        (1 in b) -> 2
        2 -> 3

cat dummy():
    obj:
        1
    hom:
        1 -> 1


b = dummy()

c = X(b)

print(1 in c)
print(2 -> 3 in c)

# EXPECT ERROR: Cannot assign variable 'b' of type 'Category'.
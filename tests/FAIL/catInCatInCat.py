cat X(a):
    obj:
        2 3 (1 in a) a
    hom:
        # (1 in b) -> 2
        2 -> 3
        a -> 3 1
        1 -> a

cat Y(b):
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
d = Y(c)

print(b in d)
# EXPECT: 0

print(b in c)
# EXPECT: 1

print(c in d)
# EXPECT: 1

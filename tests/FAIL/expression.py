cat X(a):
    obj:
        (1 and 8 or 9-6+8154) (2 in a) a (1->9 in a)
    hom:
        (1 and 8 or 9-6+8154) -> (2 in a)
        a -> (1->9 in a)
        (2 in a) -> a 

c = X(1)
print(1 in c)
print(2 -> 3 in c)

# EXPECT ERROR: Expected a variable of type category after 'in', but got 'Number'.
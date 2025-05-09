a = 1
b = 2
c = 3

cat T(a b c):
    obj:
        a b d
    hom:
        b -> c
        a -> (c+1) 

instance = T(10 20 30)

print(10 -> 20 in instance)
print(30 in instance)

# EXPECT ERROR: Undefined variable 'd'.
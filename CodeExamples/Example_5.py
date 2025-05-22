cat Categ(a b c d):
    obj:
        a b c d
    hom:
        a -> b c
        b -> c

a = 5
b = 6

d = Categ(a b -10 (a+b+9))
e = Categ((a+1) (b+1) -9 d)
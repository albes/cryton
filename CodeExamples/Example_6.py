cat C():
    obj:
        1
    hom:
    
cat Categ(a b c d):
    obj:
        a b c (5 in d) (5 -> 6 in d)
    hom:
        a -> b
        b -> c
        (5 in d) -> b
        (5 -> 6 in d) -> c

a = 5
b = 6

c = C()
d = Categ(a b -10 c)
e = Categ((a+1) (b+1) -9 d)

print 1 -> 7 in e
print 1 -> -9 in e
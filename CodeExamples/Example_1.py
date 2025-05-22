cat C():
    obj:
        1
    hom:
    
c = C()

a = 1
print(a in c)
print(1 in c)
print(a + 10 - 10 in c)

print(a + 10 - 10 -> a + 10 - 10 in c)
print(a -> a in c)
print(1 -> 1 in c)
cat C():
    obj:
        1 2 3
    hom:

c = C()

# Objects present in category
# EXPECT: 1
print(1 in c)
# EXPECT: 1
print(2 in c)
# EXPECT: 1
print(3 in c)

# Objects not present in category
# EXPECT: 0
print(0 in c)
# EXPECT: 0
print(13 in c)

# Identity morphisms (always implicitly defined)
# EXPECT: 1
print(1 -> 1 in c)
# EXPECT: 1
print(2 -> 2 in c)
# EXPECT: 1
print(3 -> 3 in c)

# Morphisms with objects not present in category
# EXPECT: 0
print(0 -> 0 in c)
# EXPECT: 0
print(1 -> 0 in c)
# EXPECT: 0
print(0 -> 1 in c)
# EXPECT: 0
print(0 -> 13 in c)
# EXPECT: 0
print(1 -> 13 in c)
# EXPECT: 0
print(13 -> 1 in c)

# Morphisms not present in category
# EXPECT: 0
print(1 -> 2 in c)
# EXPECT: 0
print(1 -> 3 in c)
# EXPECT: 0
print(2 -> 3 in c)

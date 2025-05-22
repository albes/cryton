cat Lattice():
    obj:
        0 1 2 3
    hom:
        0 -> 1
        0 -> 2
        1 -> 3
        2 -> 3

lattice = Lattice()

print(0 -> 3 in lattice)
print(1 -> 3 in lattice)
print(2 -> 1 in lattice)
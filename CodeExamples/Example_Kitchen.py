flour = 1
dough = 2
bread = 3
table = 4
sugar = 5

cat Kitchen(flour dough bread table sugar):
    obj:
        flour dough bread table sugar

    hom:
        flour -> dough
        dough -> bread
        bread -> table

kitchen = Kitchen(flour dough bread table sugar)

print(flour -> dough in kitchen)
print(dough -> bread in kitchen)
print(bread -> table in kitchen)

print(flour -> table in kitchen)

print(sugar in kitchen)
print(sugar -> table in kitchen)
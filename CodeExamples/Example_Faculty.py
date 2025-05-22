Math101 = 11
Math102 = 12
Math201 = 13
Cs101 = 21
Cs201 = 22

cat Faculty(Math101 Math102 Math201 Cs101 Cs201):
    obj:
        Math101 Math102 Math201 Cs101 Cs201
    hom:
        Math101 -> Math102
        Math102 -> Math201
        Math201 -> Cs201
        Cs101   -> Cs201

faculty = Faculty(Math101 Math102 Math201 Cs101 Cs201)

targetCourse = Cs201
prereqCourse = 0

while (prereqCourse < 300):
    if (prereqCourse -> targetCourse in faculty):
        if(prereqCourse != targetCourse):
            print(prereqCourse)
    prereqCourse = prereqCourse + 1
a = 0
b = 1
c = 5

# If branch
# ==============================
# EXPECT: 100
if (b):
    print(100)
else:
    print(101)
# ==============================

# Else branch
# ==============================
# EXPECT: 201
if (a):
    print(200)
else:
    print(201)
# ==============================

# Elif branch
# ==============================
if (a):
    print(300)
elif(b):
    # EXPECT: 301
    print(301)
else:
    print(302)
# ==============================

# Shouldn't print anything
# ==============================
if (b - b):
    print(400)
# ==============================

# Nested if statements
# ==============================
if (c):
    if (a + b):
        if (a):
            print(500)
        else:
            # EXPECT: 501
            print(501)
    # EXPECT: 503
    print(503)
# ==============================

# Multiple statements inside if
# ==============================
if (c):
    # EXPECT: 600
    print(600)
    if (a):
        print(601)
    # EXPECT: 602
    print(602)
# ==============================

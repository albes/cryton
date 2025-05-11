a = 0
b = 20

# While works correctly
# ==============================
while (a < 3):
    a = a + 1
    print(a)

# EXPECT START
# 1
# 2
# 3
# EXPECT END
# ==============================


# Shouldn't print anything
# ==============================
while (a < 3):
    print(200)
# ==============================

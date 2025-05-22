cat C():
    obj:
        1
    hom:

# EXPECT ERROR: [line 7] Error at 'if': Expect ')' after category init arguments.
c = C(if)

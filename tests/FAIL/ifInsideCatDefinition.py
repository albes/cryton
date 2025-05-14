cat T(a):
    obj:
        1 2
    hom:
        # if statements not supported inside cat definitions
        # EXPECT ERROR: [line 7] Error at 'if': Expect DEDENT after homset.
        if (1 in a):
            1 -> 2

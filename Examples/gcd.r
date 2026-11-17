// GCD of 48 and 18 - should be 6
SETC R0 INTO 48.
SETC R1 INTO 18.

// R2 = 48, R2 = 48 - 18 = 30
COPY R2 FROM R0.
SUBN R2 R1.

// All comments are first iterations
LOOP.
    // if R2 (30) > 0, then R0 = 48 - 10 = 30
    DOIF GRET R2.
        SUBN R0 R1.
    ENDN.

    // if R2 (30) < 0, then R1 = 18 - 30 (this will not run on the first loop)
    DOIF LESS R2.
        SUBN R1 R0.
    ENDN.

    // R0 = 30 (R2), then R2 = 30 - 18 = 12
    COPY R2 FROM R0.
    SUBN R2 R1.
ENNZ R2.

// 6 is a number, so convert it into ASCII (4 + 48 = 54) then print it
COPY R3 FROM R0.
ADDC R3 48.
PRNT R3.

SETC R3 INTO 10.
PRNT R3.

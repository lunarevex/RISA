// You have to start at the limit (63), though if using --unlimited you can skip it
SETC R0 INTO 63.
ADDC R0 9.       // R0 = 72 ('H')
PRNT R0.

ADDC R0 1.       // R0 = 73 ('I')
PRNT R0.

SETC R1 INTO 32. // Space is 32
PRNT R1.

SETC R0 INTO 63.
ADDC R0 14.      // R0 = 77 ('M')
PRNT R0.

ADDC R0 2.       // R0 = 79 ('O')
PRNT R0.

SUBC R0 2.       // R0 = 77 ('M')
PRNT R0.

SETC R1 INTO 10. // Newline
PRNT R1.

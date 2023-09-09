STO 0 0
STO 1 #1
STO 3 #11
STM 0 #ff
STM 1 #ff
.LOOP
    ADR 0 1 #100
    LDM 2 #100
    STM 2 #ff
    PSH 1
    POP 0
    PSH 2
    POP 1
    SUB 3 #1
    BRZ #2F
    BRN #12
HLT
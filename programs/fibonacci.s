STO 0 0
STO 1 #1
STO 3 #a
STM 0 #ff
STM 1 #ff
.LOOP
    ADM 0 1 #100
    LDM 2 #100
    STM 2 #ff
    PSH 1
    POP 0
    PSH 3
    POP 1
    SUB 3 #1
    BRZ #1F
    BRN .LOOP
HLT
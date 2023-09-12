NOP
STO 0 #0
STO 1 #0
STO 2 #f0
.LOOP
    PSH 1
    PSH 2
    RLD 3
    BRR 0 3 #1f
    STM 3 #eff7
    ADD 1 #1
    BRN #b
HLT
NOP
STO 0 #0
STO 1 #0
STO 2 #f0
STO 4 #80
.LOOP
    PSH 1
    PSH 2
    RLD 3
    BRR 0 3 #26
    STM 3 #eff7
    ADD 1 #1
    BRR 1 4 #26
    BRN #e
HLT
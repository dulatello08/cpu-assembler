NOP ; Very useful program.
STO 0 #1
STO 1 #1
.LOOP
    STM 1 #0
    STM 0 #1
    ADM 1 1 #1
    LDM 0 #0
    PSH 0
BRO .LOOP
HLT
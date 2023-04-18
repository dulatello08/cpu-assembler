NOP
STO 1 #0
.INPUT
    LDM 0 #FE
    BRR 0 1 .INPUT
NOP ; Nice routine for byte input.
.LOOP
    STM 0 #FF
    SUB 0 #1
    BRO .LOOP
HLT
NOP
STO 1 #0
.INPUT
    LDM 0 #FE
    BRR 0 1 .LABEL
NOP ; Nice routine for byte input.
HLT
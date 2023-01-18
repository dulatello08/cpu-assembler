STO 1 #0
.WAIT_FOR_INPUT:
    LDM 0 #FE
    BNR 0 1 .WAIT_FOR_INPUT
NOP ; Nice routine for byte input.
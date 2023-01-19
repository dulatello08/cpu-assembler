NOP
STO 1 #0
.INPUT
    LDM 0 #FE
    BRR 0 1 .INPUT
NOP ; Nice routine for byte input.
PSH 0 ; Print register value, should be replaced with storing to MMIO 0xFF address
.LOOP
    SUB 0 #1
    PSH 0
    BRO .LOOP
HLT
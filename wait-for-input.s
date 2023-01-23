NOP
STO 1 #0
.INPUT
    LDM 0 #FE
    BRR 0 1 .INPUT
NOP ; Nice routine for byte input.
PSH 0 ; Print register value, should be replaced with storing to MMIO 0xFF address
.LOOP
    PSH 0
    SUB 0 #1
    BRO .LOOP
HLT
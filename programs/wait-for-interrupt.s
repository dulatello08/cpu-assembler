NOP
STO 0 #f0
STO 1 #00
PSH 1
PSH 0
STM 0 #eff9
ENI ; enable int
.LOOP
    NOP
    NOP
    NOP
    BRN .LOOP
HLT
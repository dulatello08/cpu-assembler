; Echo input into display
; put keyboard_scan_to_ascii.bin at start of flash
; put keyboard_lookup_sr.s at 0xa0 offset from start of flash, put keyboard_ivt.bin at 0xc6 offset
NOP
STO 1 #ff
STO 0 #c6
PSH 0
PSH 1
STM 0 #eff9
ENI ; enable int
.WAIT
    NOP
    NOP
    NOP
    BRN .WAIT
.INT
    LDM 2 #effb
    JSR #f0a0
    STM 3 #eff7
HLT
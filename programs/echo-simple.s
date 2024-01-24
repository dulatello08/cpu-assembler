NOP; Echo input into display ; put keyboard_scan_to_ascii.bin at start of flash ; put keyboard_lookup_sr.s at 0xb0 offset from start of flash, put keyboard_ivt.bin at 0xe0 offset
STO 1 #f0
STO 0 #e0
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
    DSI
    LDM 2 #effb
    JSR #f0b0
    STM 3 #eff7
    BRN .WAIT
HLT
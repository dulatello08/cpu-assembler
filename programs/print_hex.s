CHAR_MAP_HIGH_ADDR = $0xf1
CHAR_MAP_LOW_ADDR = $0x50
.print_8bits
    ;input register 2
    ;uses registers 3 4, preserve
    STO 3 $CHAR_MAP_HIGH_ADDR
    STO 4 $CHAR_MAP_LOW_ADDR
    PSH 2
    RSH 2 #4
    ADR 2 4 #100
    LDM 4 #100
    PSH 4
    PSH 3
    RLD 2
    STM 2 #eff7
    POP 2
    STO 4 #0f
    AND 2 4
    STO 4 $CHAR_MAP_LOW_ADDR
    ADR 2 4 #100
    LDM 4 #100
    PSH 4
    PSH 3
    RLD 2
    STM 2 #eff7
    OSR
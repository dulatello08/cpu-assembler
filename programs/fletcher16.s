DATA_BLOCK_ST_ADDR_1 = $0xf0
DATA_BLOCK_ST_ADDR_2 = $0x40
DATA_BLOCK_SIZE = $0x10
RESET_VAL = $0x42
; register 0 sum1; 1 sum2.
; register 2 datablock addr1; register 3 datablock addr2
; register 4 datablock size; register 5 current counter
; register 6 to rld into;
._start
    STO 2 $DATA_BLOCK_ST_ADDR_1
    STO 3 $DATA_BLOCK_ST_ADDR_2
    STO 4 $DATA_BLOCK_SIZE
    JSR main
    STM 0 #ff
    STM 1 #ff
    HLT

.main
    STO 3 $DATA_BLOCK_ST_ADDR_2
    ADR 3 5 #100
    LDM 3 #100
    PSH 3
    PSH 2
    RLD 6
    ADR 6 0 #0x100
    BRO main_1
    STO 0 $RESET_VAL
    BRN main_1

.main_1
    LDM 0 #0x100
    XOR 1 0
    BRN main_2

.main_2
    ADD 5 #1
    BRR 4 5 main_3
    BRN main
.main_3
    OSR
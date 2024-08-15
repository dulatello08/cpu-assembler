DATA_BLOCK_ST_ADDR_1 = $0xf0
DATA_BLOCK_ST_ADDR_2 = $0x50
DATA_BLOCK_SIZE = $0x10
RESET_VAL = $0x42
; register 0 sum1; 1 sum2.
; register 2 datablock addr1; register 3 datablock addr2
; register 4 datablock size; register 5 current counter
; register 6 to rld into;
.calculate_checksum
    STO 3 $DATA_BLOCK_ST_ADDR_2
    ADR 3 5 #100
    LDM 3 #100
    PSH 3
    PSH 2
    RLD 6
    ADR 6 0 #0x100
    LDM 0 #0x100
    BRO calculate_checksum_1
    STO 0 $RESET_VAL
    BRN calculate_checksum_1

.calculate_checksum_1
    XOR 1 0
    BRN calculate_checksum_2

.calculate_checksum_2
    ADD 5 #1
    BRR 4 5 calculate_checksum_3
    BRN calculate_checksum
.calculate_checksum_3
    OSR
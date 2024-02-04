; Fahrenheit to celsius converter
.setcpu "NeoCore"
.global _start
TO_SUBTRACT = $0x20
MULTIPLICATIVE_I = $0x6f
._start
    STO 0 #64
    SUB 0 $TO_SUBTRACT ; use macro here
    STO 1 $MULTIPLICATIVE_I ; same
    MULL 0 1 0 1 ; all of 4 operands are registers
    STM 1 #ff ; dont use macro here while could
    RSH 1 #2
    MUL 1 #5 ;should have result here now
    STM 1 #0b11111111 ; use binary format, can also be ascii character or decimal but by default is hex
    BRN _halt
._halt
    HLT
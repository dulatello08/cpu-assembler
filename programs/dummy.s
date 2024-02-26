.setcpu "NeoCore"
.global _start
PRINT_R = $0xff
.loop
    SUB 0 #1
    STO 0 $PRINT_R
    BRR 0 1 loop
    OSR
._start
    STO 0 #80
    STO 1 #0
    JSR loop
    BRN _halt
._halt
    HLT
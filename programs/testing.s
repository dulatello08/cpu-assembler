._start
    STO 2 #f0
    STO 3 #80
    PSH 3
    PSH 2
    STM 0 #eff9
    ENI
    BRN loop
    HLT                      ; Halt

.loop
    NOP
    NOP
    NOP
    BRN loop

.isr_handler
    DSI
    LDM 6 #effa
    STM 6 #ff
    SUB 6 #1
    LSH 6 #1
    ADD 6 #31
    PSH 6
    PSH 2
    RLD 6
    STM 6 #eff7
    ENI
    OSR
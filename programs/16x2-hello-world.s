._start
    STO 0 #0
    STO 1 #00
    STO 2 #f1
    STO 4 #80
    BRN loop
.loop
    PSH 1
    PSH 2
    RLD 3
    BRR 0 3 _loop_exit
    STM 3 #eff7
    ADD 1 #1
    BRR 1 4 _loop_exit
    BRN .loop
._loop_exit
    HLT
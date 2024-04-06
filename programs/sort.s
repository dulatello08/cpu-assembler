._start
    STO 0 #f1
    STO 1 #1
    STO 2 #0
    LDM 5 #f100
    PSH 5
    SUB 5 #1
    PSH 5
    POP 8
    POP 5
    JSR sort_loop
    ;we have sorted array at #f101 with size of it at f100
    STO 1 #0
    BRN print_loop
.continue_start
    HLT

.print_loop
    PSH 1
    PSH 0
    RLD 2
    JSR print_8bits
    ADD 1 #1
    BRR 5 1 continue_start
    BRN print_loop
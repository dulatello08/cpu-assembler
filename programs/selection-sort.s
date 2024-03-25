; reg 0 = starting address of array
; reg 1 = current j index
; reg 2 current i index
; reg 3 temp current
; reg 4 temp next
.sort_loop
    SUB 2 #1
    BRZ exit_loop
    PSH 1
    PSH 0
    RLD 3
    ADD 1 #1
    PSH 1
    PSH 0
    RLD 4
    SBR 3 4 #ff
    BRZ after_swap
    PSH 1
    PSH 0
    RSM 3
    SUB

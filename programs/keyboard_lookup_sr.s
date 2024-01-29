NOP ; Keyboard lookup table subroutine; Input: register 2; Uses registers: 0, 1; Output: register 3
STO 0 #0
STO 1 #f0
.LOOP
    PSH 0
    PSH 1
    RLD 3
    BRR 2 3 #f0c8
    ADD 0 #2
    BRN .LOOP
ADD 0 #1
PSH 0
PSH 1
RLD 3
OSR
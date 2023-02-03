STO 2 #1        ; Initialize 2 to 1
.LOOP
    STM 2 #0
    ADR 3 1 #0      ; R3 = R1 + R2
    PSH 2
    POP 1
    PSH 3
    POP 2
    STM 2 #FF
    BRO .LOOP
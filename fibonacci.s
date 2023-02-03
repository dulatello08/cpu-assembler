STO 1 #1        ; Initialize 1 to 1
.LOOP
    ADD 1 #1
    NOP ;STM 1 #1
    NOP ;ADR 1 0 #1      ; R1 = R0 + R1
    NOP ;STM 1 #1
    NOP ;ADR 0 0 #1
    STM 1 #FF
    BRO .OVERFLOW
    BRN .LOOP
NOP
.OVERFLOW
    HLT
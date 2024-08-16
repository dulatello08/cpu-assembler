; NOT Function
; Input: 0 (value to negate)
; Output: 0 (bitwise NOT of input value)
; Uses reg 1
.bitwise_not
    STO 1 #ff
    XOR 0 1        ; XOR with 0xFF to flip all bits
    OSR

; NOT Function - 16 bit value
; Input: register 0 and 1 (value to negate), 0 being most significant.
; Output: 0, 1 (bitwise NOT of input value)
.bitwise_not_16
    PSH 1
    STO 1 #ff
    XOR 0 1        ; XOR 0 with 0xFF to flip all bits
    POP 1
    PSH 0
    STO 0 #ff
    XOR 1 0
    POP 0
    OSR

; Rotate Left (Time-Optimized)
; Input: 0 (value to rotate), 1 (number of positions)
; Output: 0 (rotated value)
; Uses register 2, memory 0x100
.rotate_left_time_optimized
    PSH 0             ; Preserve 0
    LSR 0 1 #100          ; Left shift 0 by 1 positions
    LDM 0 #100
    STO 2 #8
    STM 1 #100
    SBM 1 2 #100  ; Calculate the number of bits to wrap around (8 - reg 1)
    POP 2
    RSR 2 1 #100           ; Right shift the original value by 1 to get the wrap-around bits
    LDM 2 #100
    ORR 0 2           ; Combine the shifted and wrapped bits
    OSR
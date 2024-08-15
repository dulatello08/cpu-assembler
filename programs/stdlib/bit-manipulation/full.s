; NOT Function
; Input: R0 (value to negate)
; Output: R0 (bitwise NOT of input value)
bitwise_not:
    XOR 0, #0xFF        ; XOR with 0xFF to flip all bits
    OSR


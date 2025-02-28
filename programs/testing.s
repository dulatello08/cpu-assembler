_start:
    ; Load fixed–point values (16.16 format)
    mov 5, #3          ; A_hi = 3
    mov 6, #0x243f     ; A_lo = 0x243f
    mov 7, #2          ; B_hi = 2
    mov 8, #0xb7e1     ; B_lo = 0xb7e1

    ; Clear accumulator (R12:R13) for the final 32–bit result
    mov 12, #0
    mov 13, #0

    ;-------------------------------------------
    ; Term1: (A_hi * B_hi) << 16
    ; Compute A_hi * B_hi using smull.
    mov 5, 9          ; Copy A_hi into R9
    umull 9, 10, 7    ; Multiply R9 * B_hi; lower product in R9, upper in R10
                      ; Shifting left by 16: use low word (R9) as the high half.
    add 12, 9         ; Add term1’s high part to accumulator high (R12)
                      ; (Low part contribution is 0)

    ;-------------------------------------------
    ; Term2: A_hi * B_lo
    mov 5, 9          ; Copy A_hi into R9
    umull 9, 10, 8    ; Multiply R9 * B_lo; result: low in R9, high in R10
    add 13, 9         ; Add term2’s low word to accumulator low (R13)
    add 12, 10        ; Add term2’s high word to accumulator high (R12)

    ;-------------------------------------------
    ; Term3: A_lo * B_hi
    mov 6, 9          ; Copy A_lo into R9
    umull 9, 10, 7    ; Multiply R9 * B_hi; result in R9 (low) and R10 (high)
    add 13, 9         ; Add term3’s low word to accumulator low (R13)
    add 12, 10        ; Add term3’s high word to accumulator high (R12)

    ;-------------------------------------------
    ; Term4: (A_lo * B_lo) >> 16
    mov 6, 9          ; Copy A_lo into R9
    umull 9, 10, 8    ; Multiply R9 * B_lo; result in R9 (low) and R10 (high)
                      ; Shifting right 16 bits: use the high word (R10)
    add 13, 10        ; Add term4 (shifted) into accumulator low (R13)

    ;-------------------------------------------
    ; Final product (16.16) is in R12:R13.
    ; Move the lower 16 bits (R13) into R5 for printing.
    mov 12, 5

    ; Branch to external function that converts to hex and prints R5.
    b convert_hex

log_return:
    mov 1, #0
    bne 0, 1 actual_return
    add 0, #1
    mov 13, 5
    b convert_hex

actual_return:
    hlt
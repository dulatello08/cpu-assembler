; minimal hex-only atoi()
; Input:  r1 points to a null-terminated hex string (uppercase only)
; Output: r3 holds the converted value
; Note: Only 16-bit results are valid due to mul truncation

atoi_hex:
    psh 2            ; preserve r2
    psh 5            ; preserve r5
    psh 6            ; preserve r6

    mov 3, #0       ; initialize r3 (result) to 0

loop:
    mov 2.L, [1 + 0x1000]   ; load byte from address in r1 into r2
    mov 5, #0          ; r5 = 0
    be 2, 5, done     ; if r2 == 0 (null terminator), branch to done

    mul 3, #16      ; multiply r3 by 16 (assumes value fits in 16 bits)

    mov 5, #0x30    ; set r5 = '0'
    blt 2, 5, letter  ; if r2 < '0', it's a letter (A–F)
    mov 6, #0x39    ; set r6 = '9'
    bgt 2, 6, letter  ; if r2 > '9', treat it as letter

    sub 2, #0x30   ; convert digit ('0'-'9') to numerical value
    b add_digit

letter:
    ; convert letter ('A'-'F') to numerical value by subtracting 0x37
    sub 2, #0x37

add_digit:
    add 3, 2      ; add digit value in r2 to r3
    add 1, #1     ; advance pointer r1 to next character
    b loop        ; repeat the loop

done:
    pop 6         ; restore r6
    pop 5         ; restore r5
    pop 2         ; restore r2
    rts           ; return with result in r3


; kgets - Kernel Get String (Interrupt-Driven)
;
; Description:
;   Reads a string from the input device into a caller-provided buffer using
;   an interrupt-driven approach. The function waits (using the WFI instruction)
;   for each character to be delivered by the input ISR, which writes the character
;   to a known memory location. Input collection stops when a newline (ASCII 0x0A)
;   is received or when the maximum buffer length (minus one for the null terminator)
;   is reached. The resulting string is null-terminated.
;
; Parameters:
;   R0 - Pointer to the destination buffer.
;
; Returns:
;   The buffer pointed to by R0 is filled with the input string (null-terminated).
kgets:
    mov 1, #0xd
kgets_l:
    wfi
    b kgets
kgets_1:
    pop 1
    pop 1
    rts
type_isr:
    mov 5.L, [0x10001]
    mov [0x10000], 5.L
    be 1, 5, kgets_1
    mov [0 + 0x1000], 5.L
    add 0, #1
    rts

;------------------------------------------------------------
; Hexadecimal Conversion Subroutine
;------------------------------------------------------------
; Description:
;   Converts the 16-bit value in R5 into a 4-character
;   hexadecimal ASCII representation. For each nibble:
;     - The nibble is isolated.
;     - If the nibble is ≥ 10, 7 is added (to jump from '9' to 'A').
;     - 0x30 is added to convert it into an ASCII digit.
;     - The resulting 8-bit value is sent to UART at 0x10000.
;   After outputting 4 characters, execution branches to log_return.
;
; Registers used:
;   R1  - temporary for nibble conversion.
;   R2  - holds constant 10.
;   R5  - contains the 16-bit value to convert.

convert_hex:
    psh 1
    psh 2
    mov 2, #10           ; R2 <- 10 (for comparing nibble value)

    ;-- Process highest nibble (bits 12-15) ---------------------
    mov 5, 1             ; R1 <- R5 (make a copy)
    rsh 1, #12           ; shift right 12 bits
    and 1, #0xF          ; mask to isolate the nibble
    blt 1, 2, CH1_SKIP   ; if R1 < 10, skip adding 7
    add 1, #7            ; else, add 7 to convert (10->'A', etc.)
CH1_SKIP:
    add 1, #0x30         ; add base ASCII '0'
    mov [0x10000], 1.L   ; output the character to UART

    ;-- Process 2nd nibble (bits 8-11) --------------------------
    mov 5, 1             ; R1 <- R5
    rsh 1, #8            ; shift right 8 bits
    and 1, #0xF          ; isolate nibble
    blt 1, 2, CH2_SKIP
    add 1, #7
CH2_SKIP:
    add 1, #0x30
    mov [0x10000], 1.L

    ;-- Process 3rd nibble (bits 4-7) ---------------------------
    mov 5, 1             ; R1 <- R5
    rsh 1, #4            ; shift right 4 bits
    and 1, #0xF          ; isolate nibble
    blt 1, 2, CH3_SKIP
    add 1, #7
CH3_SKIP:
    add 1, #0x30
    mov [0x10000], 1.L

    ;-- Process lowest nibble (bits 0-3) ------------------------
    mov 5, 1             ; R1 <- R5
    and 1, #0xF          ; no shift needed
    blt 1, 2, CH4_SKIP
    add 1, #7
CH4_SKIP:
    add 1, #0x30
    mov [0x10000], 1.L
    pop 2
    pop 1
    rts
_start:
    mov 0,1, data
    jsr atoi_hex
    wfi
    hlt

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
    mov 2.L, [1 + 0]   ; load byte from address in r1 into r2
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

data:
    db "123F", 0
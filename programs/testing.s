;------------------------------------------------------------
; Euclid's GCD Algorithm (Revised)
;------------------------------------------------------------
; Description:
;   Computes the GCD of two 16-bit numbers using Euclid's
;   algorithm. The computed GCD (in R3) is then copied to R5,
;   which is passed to convert_hex.
;
; Registers used:
;   R1 - temporary for remainder computation
;   R2 - constant zero (0)
;   R3 - holds current 'a'
;   R4 - holds current 'b'
;   R5 - will receive the final GCD for conversion
;
gcd:
    mov 3, #0xaaaa      ; R3 <- 0xAAAA
    mov 4, #0xffff      ; R4 <- 0xFFFF
    mov 2, #0           ; R2 <- 0

gcd_loop:
    mov 1, 3           ; R1 <- R3, copy 'a' into R1 for modulo calculation

gcd_mod:
    blt 1, 4, mod_done ; if R1 (remainder) < R4 then we're done subtracting
    sub 1, 4           ; R1 = R1 - R4
    b gcd_mod         ; repeat until R1 < R4

mod_done:
    mov 4, 3          ; R3 <- R4, new 'a' becomes old 'b'
    mov 1, 4          ; R4 <- R1, new 'b' becomes computed remainder
    be 4, 2, gcd_done ; if R4 == 0, branch to gcd_done
    b gcd_loop        ; otherwise, continue with next iteration

gcd_done:
    mov 3, 5          ; (Spec 0x02) Copy R3 (GCD) into R5
    b convert_hex     ; Branch to convert_hex to output the hex value

log_return:
    nop
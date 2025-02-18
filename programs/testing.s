; fixed_mul:
;   16.16 fixed-point multiply.
;   Computes: result ≈ ( (A_hi * B_hi) << 16 ) + (A_hi * B_lo) + (A_lo * B_hi)
;   Input:  A in R5:R6, B in R7:R8.
;   Output: result in R5:R6.
;   Note: Ignores the high 16 bits of (A_lo*B_lo). For many cases this is acceptable.
;   Uses: R13, R14, R15 as temporaries.

fixed_mul:
    ; --- Compute temp1 = A_hi * B_hi ---
    mov 15, 5           ; R15 ← A_hi (from R5)
    mul 15, 7, 01       ; R15 ← R15 * B_hi (R7), mode 01

    ; --- Compute temp2 = A_hi * B_lo ---
    mov 14, 5           ; R14 ← A_hi (from R5)
    mul 14, 8, 01       ; R14 ← R14 * B_lo (R8), mode 01

    ; --- Compute temp3 = A_lo * B_hi ---
    mov 13, 6           ; R13 ← A_lo (from R6)
    mul 13, 7, 01       ; R13 ← R13 * B_hi (R7), mode 01

    ; --- Assemble the result ---
    ; The high product (temp1) becomes the high 16 bits of our result.
    mov 5, 15           ; R5 ← temp1
    ; Clear the low word of the result.
    mov 6, #0           ; R6 ← 0

    ; Add temp2 and temp3 into the lower half.
    add 6, 14, 01       ; R6 ← R6 + temp2
    add 6, 13, 01       ; R6 ← R6 + temp3

    ; (Optionally, one might include the contribution of (A_lo*B_lo)>>16,
    ;  but with our current mul instruction that would require extra work.)

    b end_fixed_mul

end_fixed_mul:
    nop
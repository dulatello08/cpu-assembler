_start:
    mov 5, #12       ; Compute factorial(12)
    mov 1, #1
    jsr factorial
    jsr convert_hex
    hlt

factorial:
    be 5, 1, fact_done ; Base case: if n == 1, return 1
    psh 5              ; Push n onto stack
    sub 5, #1          ; n = n - 1
    jsr factorial      ; Recursive call factorial(n-1)
    pop 6              ; Restore original n
    mul 5, 6           ; Multiply n * factorial(n-1)
fact_done:
    rts
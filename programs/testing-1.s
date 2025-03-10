;;------------------------------------------------------------
;; External Log Printing Module
;;------------------------------------------------------------
;; Description:
;;   Prints a null-terminated string to the UART at address 0x10000.
;;   Expects:
;;       R10 - pointer to the string to print.
;;   Uses:
;;       R1  - temporary register for the current character.
;;   Behavior:
;;       Loads each 8-bit character from memory, sends it to the UART,
;;       increments the pointer, and repeats until a null (0) is found.
;;       Upon completion, branches to a hardcoded label 'log_return'.
;;
;; Note:
;;   Since we do not have subroutine support yet, the return is hardwired.
;;   Make sure your main program defines the label 'log_return' so that
;;   control returns properly.
;;------------------------------------------------------------
;
print_log:
    mov 1.L, [10 + 0x00]    ; Load 8-bit char from address in R10 into R1.L
    be 1, 0, log_return     ; If R1 equals 0 (null terminator), branch to log_return
    mov [0x10000], 1.L      ; Output the character to UART
    add 10, #1              ; Increment pointer in R10
    b print_log             ; Loop back to print the next character
log_return:
    rts
;
;;------------------------------------------------------------
;; Hexadecimal Conversion Subroutine
;;------------------------------------------------------------
;; Description:
;;   Converts the 16-bit value in R5 into a 4-character
;;   hexadecimal ASCII representation. For each nibble:
;;     - The nibble is isolated.
;;     - If the nibble is ≥ 10, 7 is added (to jump from '9' to 'A').
;;     - 0x30 is added to convert it into an ASCII digit.
;;     - The resulting 8-bit value is sent to UART at 0x10000.
;;   After outputting 4 characters, execution branches to log_return.
;;
;; Registers used:
;;   R1  - temporary for nibble conversion.
;;   R2  - holds constant 10.
;;   R5  - contains the 16-bit value to convert.
;
;convert_hex:
;    mov 2, #10           ; R2 <- 10 (for comparing nibble value)
;
;    ;-- Process highest nibble (bits 12-15) ---------------------
;    mov 5, 1             ; R1 <- R5 (make a copy)
;    rsh 1, #12           ; shift right 12 bits
;    and 1, #0xF          ; mask to isolate the nibble
;    blt 1, 2, CH1_SKIP   ; if R1 < 10, skip adding 7
;    add 1, #7            ; else, add 7 to convert (10->'A', etc.)
;CH1_SKIP:
;    add 1, #0x30         ; add base ASCII '0'
;    mov [0x10000], 1.L   ; output the character to UART
;
;    ;-- Process 2nd nibble (bits 8-11) --------------------------
;    mov 5, 1             ; R1 <- R5
;    rsh 1, #8            ; shift right 8 bits
;    and 1, #0xF          ; isolate nibble
;    blt 1, 2, CH2_SKIP
;    add 1, #7
;CH2_SKIP:
;    add 1, #0x30
;    mov [0x10000], 1.L
;
;    ;-- Process 3rd nibble (bits 4-7) ---------------------------
;    mov 5, 1             ; R1 <- R5
;    rsh 1, #4            ; shift right 4 bits
;    and 1, #0xF          ; isolate nibble
;    blt 1, 2, CH3_SKIP
;    add 1, #7
;CH3_SKIP:
;    add 1, #0x30
;    mov [0x10000], 1.L
;
;    ;-- Process lowest nibble (bits 0-3) ------------------------
;    mov 5, 1             ; R1 <- R5
;    and 1, #0xF          ; no shift needed
;    blt 1, 2, CH4_SKIP
;    add 1, #7
;CH4_SKIP:
;    add 1, #0x30
;    mov [0x10000], 1.L
;    rts
;
;;-------------------------------------------------
;; Subroutine: fib
;; Description: Computes Fibonacci(n)
;;   - Base Case: if n < 2, returns n (i.e. 0 or 1)
;;   - Recursive Case: fib(n) = fib(n-1) + fib(n-2)
;; Registers used:
;;   R5 holds the input n and later the result.
;;   R1 is used to save the original n.
;;   R3 is used to store the result of fib(n-1).
;;-------------------------------------------------
;
;fib:
;    mov 2, #2          ; Load immediate 2 into R2 for comparison
;    blt 5, 2, fib_base ; If R5 (n) is less than 2, branch to base case
;
;    ;--- Recursive case: compute fib(n-1) ---
;    mov 5, 1           ; Copy current n (R5) into R1 (save original n)
;    psh 1              ; Push R1 onto the stack to preserve its value
;    sub 5, #1          ; Compute (n - 1); result stored in R5
;    jsr fib            ; Recursive call: compute fib(n-1); result in R5
;    mov 5, 3           ; Save result fib(n-1) from R5 into R3
;    pop 1              ; Restore original n from the stack into R1
;
;    ;--- Compute fib(n-2) ---
;    psh 3              ; Push fib(n-1) (R3) to preserve it across the next call
;    mov 1, 5           ; Restore original n from R1 back into R5
;    sub 5, #2          ; Compute (n - 2); result stored in R5
;    jsr fib            ; Recursive call: compute fib(n-2); result in R5
;    pop 3              ; Restore fib(n-1) from the stack into R3
;
;    ;--- Combine results ---
;    add 5, 3           ; Add fib(n-2) (R5) and fib(n-1) (R3); result in R5
;    rts                ; Return from subroutine
;
;fib_base:
;    ;--- Base case: when n is 0 or 1 ---
;    rts                ; Return from subroutine with R5 unchanged
.print
    STO 5 #0                ; Initialize counter register R5 to 0
    BRN print_loop          ; Branch to the start of the print loop

.print_loop
    PSH 3
    PSH 2
    RLD 1                   ; Load the byte at the address of R2 and R3 into R1
    STM 1 #eff7             ; Store the byte from R1 to the peripheral control register for output
    ADD 5 #1                ; Increment counter in R5 by 1
    ADD 3 #1                ; Increment current address in R3 by 1
    BRR 4 5 print_exit      ; If the counter (R5) equals the size of the string (R4), exit the loop
    BRN print_loop          ; Otherwise, branch back to the start of the loop

.print_exit
    OSR                     ; Return from the subroutine
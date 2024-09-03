; (Not actually merge sort) Bubble sort implementation for NeoCore 8x16 CPU
; Registers:
;   0 = Starting address of the array
;   1 = Current index j in the inner loop
;   2 = Current index i in the outer loop
;   3 = Temporary storage for the current element
;   4 = Temporary storage for the next element
;   5 = Total number of items in the array
;   6 = Miscellaneous/temporary storage
;   7 = Zero register (used for comparisons)
;   8 = Total number of items squared (outer loop limit)

.sort_loop
    ; Reset inner loop index
    STO 1 #1
    BRN inner_loop

.inner_loop
    ; Load current element
    PSH 1
    PSH 0
    RLD 3

    ; Load next element
    ADD 1 #1
    PSH 1
    PSH 0
    RLD 4

    ; Compare and swap if necessary
    SBR 3 4 #101
    BRZ skip_swap

    ;Gotta swap
    PSH 1
    PSH 0
    RSM 3
    SUB 1 #1
    PSH 1
    PSH 0
    RSM 4
    BRN no_swap

.skip_swap
    SUB 1 #1
    BRN no_swap

.no_swap
    ; Increment inner loop index and check if it's time to reset
    ADD 1 #1
    SBR 5 1 #102
    LDM 6 #102
    BNR 6 7 inner_loop  ; Continue inner loop if R1 < R5
    ; exited inner loop here
    ; Increment outer loop index and check if sorting is complete
    ADD 2 #1
    BRR 2 8 exit  ; Exit if R2 = R8
    BRN sort_loop

.exit
    OSR
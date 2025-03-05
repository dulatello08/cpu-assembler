_start:
    mov 1,2, ivt
    mov [0x20002], 2
    mov 5, #1
    mov [0x20004] 5

loop:
    nop
    nop
    b loop

ivt:
    db label

label:
    nop
    hlt
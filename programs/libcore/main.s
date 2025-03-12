_start:
    mov 1,2, ivt
    mov [0x20002], 2
    mov 5, #1
    mov [0x20004], 5
    jsr kgets
    mov 1, #0x0
    jsr atoi_hex
    mov 0, #0x0a
    mov [0x10000], 0.L
    mov 0, #0x0
    mov 3, 5
    rsh 5, #4
    jsr convert_hex
    hlt
ivt:
    db type_isr
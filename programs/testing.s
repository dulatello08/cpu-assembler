_start:
    mov 5, #12
    mov [0x20004] 5
    hlt
ivt:
    db 0x
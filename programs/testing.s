_start:
    mov 0, #0x62
    psh 0
    pop 1
    mov [#0x10000], 1
    hlt
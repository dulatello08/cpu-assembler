_start:
    mov 1, #0xa455
    mov 1, 0
    sub 1, #0x455
    mov [0x7000], 1, 0

    mov 2, #0x7000
    mov 3, 4 [2]
_start:
    mov 1,2, ivt
    mov [0x20002], 2
    mov 5, #1
    mov [0x20004] 5
    wfi
    hlt
ivt:
    db label

label:
    mov 0, 10, helloworld
    jsr print_log
    rts
helloworld:
    db "Hello, World!", 0
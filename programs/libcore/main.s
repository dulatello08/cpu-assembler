_start:
    mov 1, 2, ivt
    mov [0x20002], 2
    mov 5, #1
    mov [0x20004], 5
    mov 0, 10, exit
main:
    mov 0, #0
    mov 5, #0
    jsr kgets
    mov 0, 11
    jsr strcmp
    mov 1, #0
    be 1, 5, handle_exit
    jsr atoi_hex
    jsr print_newline
    mov 3, 5
    rsh 5, #4
    jsr convert_hex
    jsr print_newline
    b main
handle_exit:
    hlt
ivt:
    db type_isr
exit:
    db "exit", 0
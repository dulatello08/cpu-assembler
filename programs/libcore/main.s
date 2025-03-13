_start:
    mov 1,2, ivt
    mov [0x20002], 2
    mov 5, #1
    mov [0x20004], 5
main:
    mov 0, #0
    mov 5, #0
    jsr kgets
    jsr atoi_hex
    jsr print_newline
    mov 3, 5
    rsh 5, #4
    jsr convert_hex
    jsr print_newline
    b main
ivt:
    db type_isr
_start:
    mov 1,2, ivt
    mov [0x20002], 2
    mov 5, #1
    mov [0x20004], 5
    mov 1, #0xd
    mov 2, #0
loop:
    wfi
    b loop
command_entered:
    mov 10, #0
    jsr print_log
    hlt
ivt:
    db type

type:
    mov 5.L, [0x10001]
    be 1, 5, command_entered
    mov [2 + 0xD0000], 5.L
    add 2, #1
    rts
;type_cont:
;    mov 0, 10, helloworld
;    jsr print_log
;    rts
;    mov 0, 10, helloworld
;    mov 0, 11, helloworld1
;    jsr strcmp
;    jsr convert_hex
;    hlt
;help:
;    db "help", 0
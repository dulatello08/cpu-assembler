print_reset:
    psh 1
    mov 1, #0x1B       ; ESC
    mov [0x10000], 1.L
    mov 1, #0x5B       ; '['
    mov [0x10000], 1.L
    mov 1, #0x30       ; '0'
    mov [0x10000], 1.L
    mov 1, #0x6D       ; 'm'
    mov [0x10000], 1.L
    pop 1
    rts
print_red:
    psh 1
    mov 1, #0x1B       ; ESC
    mov [0x10000], 1.L
    mov 1, #0x5B       ; '['
    mov [0x10000], 1.L
    mov 1, #0x33       ; '3'
    mov [0x10000], 1.L
    mov 1, #0x31       ; '1'  --> "31" sets red foreground
    mov [0x10000], 1.L
    mov 1, #0x6D       ; 'm'
    mov [0x10000], 1.L
    pop 1
    rts
print_green:
    psh 1
    mov 1, #0x1B       ; ESC
    mov [0x10000], 1.L
    mov 1, #0x5B       ; '['
    mov [0x10000], 1.L
    mov 1, #0x33       ; '3'
    mov [0x10000], 1.L
    mov 1, #0x32       ; '2'  --> "32" sets green
    mov [0x10000], 1.L
    mov 1, #0x6D       ; 'm'
    mov [0x10000], 1.L
    pop 1
    rts
print_blue:
    psh 1
    mov 1, #0x1B       ; ESC
    mov [0x10000], 1.L
    mov 1, #0x5B       ; '['
    mov [0x10000], 1.L
    mov 1, #0x33       ; '3'
    mov [0x10000], 1.L
    mov 1, #0x34       ; '4'  --> "34" sets blue
    mov [0x10000], 1.L
    mov 1, #0x6D       ; 'm'
    mov [0x10000], 1.L
    pop 1
    rts
print_yellow:
    psh 1
    mov 1, #0x1B       ; ESC
    mov [0x10000], 1.L
    mov 1, #0x5B       ; '['
    mov [0x10000], 1.L
    mov 1, #0x33       ; '3'
    mov [0x10000], 1.L
    mov 1, #0x33       ; '3'  --> "33" sets yellow
    mov [0x10000], 1.L
    mov 1, #0x6D       ; 'm'
    mov [0x10000], 1.L
    pop 1
    rts
_start:
    mov 2, data_string
    mov 3,  #13

_loop:
    mov 1.L,[2+0x0]
    mov [0x10000],1.L
    add 2,#0x1
    sub 3,#0x1
    bne 3,0,_loop
    b _start

data_string:
    db "Hello World!\n"
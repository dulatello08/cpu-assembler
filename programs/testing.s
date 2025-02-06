_start:
    nop
    b loop1
loop2:
    sub 1 #1
    b _start
    b loop3
loop3:
    nop
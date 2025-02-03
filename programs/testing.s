_start:
    nop
    mov 1, #10           ; move immediate value 10 into register 1
    mov 2, #20           ; move immediate value 20 into register 2
    
    add 3, 1             ; add contents of register 1 to register 3, result in register 3
    sub 4, #5            ; subtract immediate 5 from register 4
    mul 5, 2             ; multiply register 5 by contents of register 2, result in register 5
    and 6, [#0xFF]        ; bitwise AND register 6 with immediate 0xFF
    or  7, 3             ; bitwise OR of register 7 and register 3, result in register 7
    xor 8, 4             ; bitwise XOR of register 8 and register 4, result in register 8
    lsh 9, #2            ; logical shift left register 9 by 2 bits
    rsh 10, 11           ; logical shift right register 10 by the amount in register 11
    
    mov 0.L, [0x3000]    ; move lower part of register 0 from memory at address 0x3000
    mov [0x3000], 0.H    ; move high part of register 0 into memory at address 0x3000
    
    mov 1, [2 + #4]      ; move value from memory at address (register 2 + 4) to register 1
    mov [2 + #8], 3, 4   ; move value from register 3 using base register 2 with offset 8, additional register 4 involvement
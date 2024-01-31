NOP ; estimate celsius from fahrenheit temp.
STO 0 #64
SUB 0 #20
STO 1 #6f
MULL 0 1 0 1
; here 0 is lower part of result of mult while 1 is higher part. in total they would be 16 bit int but they are in separate registers. i need to perform right shift on that 16 bit int but I have RSH opcode that can only perform shift on 1 register
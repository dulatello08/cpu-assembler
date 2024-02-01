NOP ; estimate celsius from fahrenheit temp.
STO 0 #64
SUB 0 #20
STO 1 #6f
MULL 0 1 0 1
RSH 1 #2
MUL 1 #5 ;should have result here now
STM 1 #ff
HLT
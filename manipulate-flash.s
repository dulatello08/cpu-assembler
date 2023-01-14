NOP ; Reads flash data from addresses specified in Rd and Rn.
STO 0 #0
STO 1 #2C
RDM 1 0 #0
STO 1 #80
STM 1 #2D
PRT 0
HLT
NOP ; Reads flash data from addresses specified in Rd and Rn.
STO 0 #0
STO 1 #F
RDM 1 0 #0 
STO 1 #80
STM 1 #F 
PRT 0
HLT
STO 0, 1      ;Initialize memory location 0 to 1
STO 1, 0      ;Initialize memory location 1 to 0
STM 1, 0      ;Store the value of memory location 0 in memory location 1
NOP
.loop:
  ADM 0, 1, 1  ;memory location 0 = memory location 1 + memory location 1
  PSH 1
  STM 1, 0     ;store the value of memory location 0 in memory location 1
  ADM 1, 0, 0  ;memory location 1 = memory location 0 + memory location 0
  NOP ; repeat this loop for as many times as desired before the overflow
BRO .loop
HLT
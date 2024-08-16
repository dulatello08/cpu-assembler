._start
    STO 0 #b2
    STO 1 #5
    JSR rotate_left_time_optimized
    STM 0 #ff                ; Store result in memory
    HLT                      ; Halt
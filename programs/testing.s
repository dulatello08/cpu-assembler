._start
    STO 2 #f0              ; Initialize addr1
    STO 3 #50              ; Initialize addr2
    STO 4 #10              ; Initialize block size
    JSR calculate_checksum   ; Call the checksum subroutine
    STM 0 #ff                ; Store sum1 result in memory
    STM 1 #ff                ; Store sum2 result in memory
    HLT                      ; Halt
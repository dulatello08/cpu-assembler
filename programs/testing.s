;-----------------------------------------------------------
; Revised CRC-16/CCITT-FALSE Example with Hardcoded Data
; Computes the CRC over "Hello World!" (12 bytes) then prints 
; the computed 16-bit CRC (in hex) to UART at address 0x10000.
; 
; Registers used:
;   R0: Zero constant (initialized to 0)
;   R1: Temporary for loading a data byte (using its .L field)
;   R2: Data pointer (points to current byte in data_string)
;   R3: Byte counter (number of bytes remaining)
;   R4: CRC accumulator (initialized to 0xFFFF)
;   R5: Bit-loop counter (set to 8 for each byte)
;   R6: Temporary for testing the MSB of R4
;   R7: Working copy of CRC (for conversion to ASCII)
;   R8: Nibble counter (set to 4)
;   R9: Temporary for nibble extraction and conversion
;   R10: Constant (loaded with 10 for comparison)
;-----------------------------------------------------------

_start:
    mov 0, #0              ; R0 ← 0 (initialize zero constant)

    ;-- Set up pointer to data and length --
    ; Load the 32-bit address of data_string into R3 and R2.
    mov 3, 2, data_string  ; R3:upper, R2:lower half of data_string address
    mov 3, #12             ; R3 ← 12 (the number of bytes in "Hello World!")

    ;-- Initialize CRC accumulator --
    mov 4, #0xFFFF         ; R4 ← 0xFFFF

    b crc_compute          ; Branch to the CRC computation routine


;-----------------------------------------------------------
; CRC Computation Routine (CRC-16/CCITT-FALSE)
; For each byte:
;   1. Clear R1, load an 8-bit value from [R2+0x0] into R1.L.
;   2. Shift R1 left by 8 (placing the byte in the high half).
;   3. XOR that value into R4.
;   4. Process 8 iterations of:
;         if (R4 & 0x8000) then R4 = (R4 << 1) XOR 0x1021
;         else                R4 = (R4 << 1)
;   5. Increment R2 and decrement R3.
;-----------------------------------------------------------
crc_compute:
crc_loop:
    bne 3, 0, process_byte ; if R3 ≠ R0, process a byte
    b print_crc            ; else, all bytes processed—go print the CRC

process_byte:
    mov 1, #0              ; Clear R1 (set R1 to 0)
    mov 1.L, [2+0x0]       ; Load one byte from memory at address in R2 into R1.L
    lsh 1, #8             ; Shift R1 left by 8 bits (byte moves to high 8 bits)
    xor 4, 1              ; R4 ← R4 XOR R1 (update CRC accumulator)

    mov 5, #8             ; Set bit-loop counter R5 to 8

bit_loop:
    mov 6, 4              ; Copy current CRC (R4) into R6 for testing its MSB
    and 6, #0x8000        ; R6 ← R6 AND 0x8000 (mask the MSB)
    bne 6, 0, bit_poly     ; If R6 ≠ R0, branch to bit_poly
    lsh 4, #1             ; Else, shift R4 left by 1
    sub 5, #1             ; Decrement bit-loop counter R5
    bne 5, 0, bit_loop     ; If R5 ≠ R0, repeat bit_loop
    add 2, #1             ; Increment pointer R2 (move to next byte)
    sub 3, #1             ; Decrement byte counter R3
    bne 3, 0, crc_loop     ; If R3 ≠ R0, loop to process next byte
    b print_crc           ; Else, all bytes done—branch to print_crc

bit_poly:
    lsh 4, #1             ; Shift R4 left by 1
    xor 4, #0x1021        ; R4 ← R4 XOR 0x1021 (apply polynomial)
    sub 5, #1             ; Decrement bit-loop counter R5
    bne 5, 0, bit_loop     ; If R5 ≠ R0, continue bit_loop


;-----------------------------------------------------------
; CRC-to-ASCII Conversion & Printing Routine
; Converts the 16-bit CRC in R4 into 4 hex digits (ASCII)
; and outputs each digit to UART at address 0x10000.
;-----------------------------------------------------------
print_crc:
    mov 7, 4              ; Copy computed CRC from R4 into working register R7
    mov 8, #4             ; Set nibble counter R8 to 4

print_loop:
    mov 9, 7              ; Copy working CRC (R7) into R9 for nibble extraction
    rsh 9, #12            ; Right-shift R9 by 12 bits (top nibble now in lower 4 bits)
    mov 10, #10           ; Load constant 10 into R10
    blt 9, 10, less_than_ten  ; If R9 < R10, branch to less_than_ten
    add 9, #0x37          ; Else, add 0x37 to convert nibble (10-15) to 'A'-'F'
    b store_digit
less_than_ten:
    add 9, #0x30          ; Add 0x30 to convert nibble (0-9) to '0'-'9'
store_digit:
    mov [0x10000], 9.L    ; Output the ASCII digit (from R9.L) to UART at address 0x10000
    lsh 7, #4             ; Shift working CRC (R7) left by 4 to discard the printed nibble
    sub 8, #1             ; Decrement nibble counter R8
    bne 8, 0, print_loop  ; If R8 ≠ R0, repeat print_loop
    nop                   ; End of program


;-----------------------------------------------------------
; Data Section
;-----------------------------------------------------------
data_string:
    db "Hello World!"
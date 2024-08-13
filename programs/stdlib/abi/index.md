# NeoCore 8x16 ABI Documentation

## Introduction

This document outlines the Application Binary Interface (ABI) for the NeoCore 8x16 microkernel. The ABI defines how software components, including the kernel, standard library, and user applications, interact at the binary level. It specifies register usage, calling conventions, memory layout, and other essential details for consistent and reliable system operation.

## Register Conventions

### General-Purpose Registers

- **R0**: General-purpose register, often used for returning values.
- **R1**: General-purpose register, commonly used for secondary return values or intermediate calculations.
- **R2**: General-purpose register, often used for the first part of a 16-bit memory address or data block starting address.
- **R3**: General-purpose register, typically used for the second part of a 16-bit memory address or additional data handling.
- **R4**: General-purpose register, used for passing the size of data blocks or arrays.
- **R5**: General-purpose register, used as a counter or loop variable.
- **R6**: General-purpose register, reserved for special operations like loading data from complex memory addresses.
- **R7**: Reserved for future use or special purposes.

### Special Registers

None for now

## Calling Conventions

### Function Call Convention

- **Function Entry**:
    - The caller must save any registers that it needs to preserve across function calls.
    - Arguments are to be passed in the registers `R2` to `R5` depending on the function requirements.

- **Function Return**:
    - The return value is placed in `R0`. If a function returns multiple values, additional values are placed in `R1`, or caller and callee agree on a memory block for output/result.
    - The callee must restore any registers it modifies, except for `R0` and `R1`, which are used for return values (more exceptions apply).

### Stack Usage

- **Stack Management**:
    - `PSH` (Push) operations store a value on the stack.
    - `POP` (Pop) operations return value from the stack and remove the value from the stack.

- **Stack Usage in Functions**:
    - The stack can be used to quickly store local variables(register values) and pass arguments if registers alone are insufficient.
    - The function should clean up the stack before returning, by popping values.

## Memory Layout

### Memory Segments

- **Boot Sector**:
    - The boot sector contains everything inside the _start routine.  
Here is the memory map  
Address range is including start address but excluding end address.

| Address Range   | Memory Space                            |
|-----------------|-----------------------------------------|
| 0x0000 - 0x00~~ | Boot Sector (max 256 bytes)             |
| 0x00~~ - 0xEFF3 | Usable Memory (min 61,174 bytes)        |
| 0xEFF3 - 0xEFF4 | Flags                                   |
| 0xEFF4 - 0xEFF3 | Stack Memory                            |
| 0xEFF3 - 0xEFF7 | MMU Control                             |
| 0xEFF7 - 0xEFFF | Peripheral Control                      |
| 0xEFFF - 0xEFFF | Memory Block                            |
| 0xF000 - 0xFFFF | Reserved for Flash Memory (4,097 bytes) |
### Addressing Conventions

- **16-bit Addressing**:
    - Memory addresses are 16-bit values, however data stored at a memory address is an 8 bit-value.
    - Direct memory operations should use the `LDM` (Load Memory) and `STM` (Store Memory) instructions with appropriate addressing via immediate address.
    - Indirect memory operations should use the `RLD` (Load relative memory) and `RSM` (Store relative memory) instructions with address being 2 topmost values on the stack in order of HI to LO value, e.g `PSH 0xf0; PSH 0x00` and then calling `RLD 1` will load value from address `0xf000` in register 1.

## Interrupt Handling

### Maskable Interrupts

- **ENI (Enable Interrupts)**: Enables maskable interrupts.
- **DSI (Disable Interrupts)**: Disables maskable interrupts.

### Non-Maskable Interrupts

- Non-maskable interrupts cannot be disabled and are reserved for critical hardware events.

### Interrupt Service Routine (ISR)

- **Register Saving**: The ISR should save all registers that it modifies, typically by pushing them onto the stack at the beginning of the ISR and popping them before returning.
- **Interrupt Return**: Use the `OSR` (Out of SubRoutine) instruction to return from an interrupt.

## Example Usage

### Simple Subroutine Example

```assembly
._start
    STO 2 $0xf0              ; Initialize addr1
    STO 3 $0x50              ; Initialize addr2
    STO 4 $0x10              ; Initialize block size
    STO 5 $0x00              ; Initialize counter to 0
    JSR calculate_checksum   ; Call the checksum subroutine
    STM 0 #ff                ; Store sum1 result in memory
    STM 1 #ff                ; Store sum2 result in memory
    HLT                      ; Halt
```

### Conclusion

This ABI is designed to facilitate consistent and efficient communication between different software components in the NeoCore 8x16 environment. As the microkernel and associated libraries evolve, this document will be updated to reflect any changes or additions to the ABI.
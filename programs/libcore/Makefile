# Paths to tools
AS = /Users/dulat/Documents/cpu/cpu-assembler/nc16x32-as
LD = /Users/dulat/Documents/cpu/cpu-assembler/nc16x32-ld

# Output executable
OUTPUT = a.out
EMULATOR_OUTPUT = /Users/dulat/Documents/cpu/cpu-emulator/$(OUTPUT)

# Find all .s files in the directory
SRC = $(wildcard *.s)
OBJ = $(SRC:.s=.o)

# Default target: build and copy
all: $(OUTPUT)
	cp $(OUTPUT) $(EMULATOR_OUTPUT)

# Assemble each .s file into an .o file
%.o: %.s
	$(AS) -i $< -o $@

# Link all object files into the final executable
$(OUTPUT): $(OBJ)
	$(LD) $(OBJ)

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(OUTPUT)

.PHONY: all clean
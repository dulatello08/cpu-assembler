# Compiler settings
CXX = clang++
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++20 -O2 -I. -g
LDFLAGS = -v

# Project files
ASSEMBLER_SOURCES = assembler/assembler.cpp assembler/lexer.cpp assembler/parser.cpp
LINKER_SOURCES = linker/linker.cpp linker/object_files_parser.cpp linker/memory_layout.cpp
ASSEMBLER_OBJECTS = $(ASSEMBLER_SOURCES:.cpp=.o)
LINKER_OBJECTS = $(LINKER_SOURCES:.cpp=.o)
ASSEMBLER_EXECUTABLE = nc8x16-as
LINKER_EXECUTABLE = nc8x16-ld

# Target rules
all: $(ASSEMBLER_EXECUTABLE) $(LINKER_EXECUTABLE)

$(ASSEMBLER_EXECUTABLE): $(ASSEMBLER_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(LINKER_EXECUTABLE): $(LINKER_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(ASSEMBLER_OBJECTS) $(LINKER_OBJECTS) $(ASSEMBLER_EXECUTABLE) $(LINKER_EXECUTABLE)

.PHONY: all clean

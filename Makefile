# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++20 -O0 -I. -g
LDFLAGS =

# Project files
ASSEMBLER_SOURCES = assembler/assembler.cpp assembler/lexer.cpp assembler/parser.cpp assembler/util.cpp
LINKER_SOURCES = linker/linker.cpp linker/object_files_parser.cpp linker/memory_layout.cpp
ASSEMBLER_OBJECTS = $(ASSEMBLER_SOURCES:.cpp=.o)
LINKER_OBJECTS = $(LINKER_SOURCES:.cpp=.o)
ASSEMBLER_EXECUTABLE = nc16x32-as
LINKER_EXECUTABLE = nc16x32-ld

# Target rules
all: $(ASSEMBLER_EXECUTABLE) $(LINKER_EXECUTABLE)

$(ASSEMBLER_EXECUTABLE): $(ASSEMBLER_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(LINKER_EXECUTABLE): $(LINKER_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

assembler/machine_description.h: config/neocore16x32.mdesc parse_md.py
	./parse_md.py

%.o: %.cpp assembler/machine_description.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(ASSEMBLER_OBJECTS) $(LINKER_OBJECTS) $(ASSEMBLER_EXECUTABLE) $(LINKER_EXECUTABLE)

.PHONY: all clean

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -I.
LDFLAGS =

# Project files
SOURCES = main.cpp assembler.cpp conf_parser.cpp lexer.cpp parser.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = assembler

# Target rules
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
CC = gcc
CFLAGS = -Wall -Wextra -Werror -O0 -I. -g

all: assembler

assembler: main.o assembler.o
	$(CC) $(CFLAGS) $^ -o assembler

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

assembler.o: assembler.c
	$(CC) $(CFLAGS) -c assembler.c -o assembler.o

clean:
	rm -f *.o assembler

.PHONY: all clean
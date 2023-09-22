CC = gcc
CFLAGS = -Wall -Wextra -Werror -O0

all: configurator

configurator: configurator.o
	$(CC) $(CFLAGS) $^ -o configurator -L/opt/homebrew/lib/ -lyaml

configurator.o: main.c
	$(CC) $(CFLAGS) -I/opt/homebrew/include/ -c main.c -o configurator.o
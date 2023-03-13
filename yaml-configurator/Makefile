CC = gcc
CFLAGS = -Wall -Wextra -Werror -O0 -I. -g

all: configurator

configurator: configurator.o
	$(CC) $(CFLAGS) $^ -o configurator

configurator.o: main.c
	$(CC) $(CFLAGS) -c main.c -o configurator.o
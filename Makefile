CC := gcc
CFLAGS := -Wextra -Wall -Os
LDFLAGS := -s -static


default: all

init: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o init

all: init

CC := gcc
CFLAGS := -Wextra -Wall -Os
LDFLAGS := -s -static


default: all

init:
	$(CC) $(CFLAGS) $(LDFLAGS) main.c -o init
	
.PHONY: clean install all 

clean:
	rm init

install:
	cp /root/init/stinit/init /root/boot/disk/sbin/init

all: init

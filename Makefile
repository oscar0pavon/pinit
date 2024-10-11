CC := gcc
CFLAGS := -Wextra -Wall -Os
LDFLAGS := -s -static


default: all

init:
	$(CC) -std=c11 -pthread $(CFLAGS) $(LDFLAGS) main.c -o init -lpthread
	
.PHONY: clean install all 

clean:
	rm init

install:
	cp /root/init/stinit/init /root/boot/disk/sbin/init
	#cp /root/init/stinit/init /sbin/pinit

all: init

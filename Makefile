CFLAGS := -Wextra -Wall -Os
LDFLAGS := -s -static


pinit: main.c
	cc -std=c11 -pthread $(CFLAGS) $(LDFLAGS) main.c -o pinit -lpthread
	
clean:
	rm -f pinit

install:
	cp pinit /root/virtual_machine/disk/sbin/pinit

release:
	cp pinit /sbin/pinit


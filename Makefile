CFLAGS := -Wextra -Wall -Os
LDFLAGS := -s -static


pinit: main.c
	cc -std=gnu99 -pthread $(CFLAGS) $(LDFLAGS) main.c -o pinit -lpthread
	
clean:
	rm -f pinit

install:
	cp pinit /root/virtual_machine/disk/sbin/pinit

release:
	cp -f pinit /pinit


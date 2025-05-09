CC=musl-gcc
CFLAGS := -Wextra -Wall -Os
LDFLAGS := -s -static


pinit: main.c
	$(CC) -std=gnu99 -pthread $(CFLAGS) $(LDFLAGS) main.c -o pinit -lpthread
	upx pinit
	
	
clean:
	rm -f pinit

install:
	cp -f pinit /pinit



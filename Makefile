CC=cc
CFLAGS=-Wall -Werror -Wextra

default: ls
all: default

ls: cmp.o ls.o
	$(CC) $(CFLAGS) -o ls cmp.o ls.o -lm

ls.o: ls.c ls.h
	$(CC) $(CFLAGS) -c ls.c

cmp.o: cmp.c cmp.h
	$(CC) $(CFLAGS) -c cmp.c

clean:
	rm -rf $(TARGET) *.o 
CC=cc
CFLAGS=-Wall -Werror -Wextra

ls: ls.c
	$(CC) $(CFLAGS) -o ls ls.c
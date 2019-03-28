CC=gcc
CFLAGS=-I

client: admin.c
	$(CC) -o client admin.c

server: keeper.c
	$(CC) -o server keeper.c
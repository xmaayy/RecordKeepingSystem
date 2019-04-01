CC=gcc
CFLAGS=-Wall

client: admin.o record.o
	$(CC) -o client admin.o record.o

server: keeper.o record.o
	$(CC) -o server keeper.o record.o

keeper.o: keeper.c record.h
	$(CC) $(CFLAGS) -c keeper.c

admin.o: admin.c record.h
	$(CC) $(CFLAGS) -c admin.c

record.o: record.c record.h
	$(CC) $(CFLAGS) -c record.c

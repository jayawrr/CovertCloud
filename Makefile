CC = gcc
CFLAGS = -c -g

all: covert_client covert_server covert_tester

covert_client: covert_client.o covert.o
		$(CC) -o covert_client covert_client.o covert.o

covert_server: covert_server.o covert.o
		$(CC) -o covert_server covert_server.o covert.o

covert_tester: covert_tester.o covert.o
		$(CC) -o covert_tester covert_tester.o covert.o

clean:
		rm -f *.o

%.o:	%.c
		$(CC) $(CFLAGS) $*.c

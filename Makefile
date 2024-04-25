CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all: server client admin1

server : server.o utils_v1.o network.o game.o ipc.o
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o network.o game.o ipc.o

client : client.o utils_v1.o network.o
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o network.o

admin1: admin1.o utils_v1.o ipc.o
	$(CC) $(CCFLAGS) -o admin1 admin1.o utils_v1.o ipc.o

server.o: server.c utils_v1.h messages.h network.h game.h ipc.h
	$(CC) $(CCFLAGS) -c server.c

client.o: client.c utils_v1.h messages.h network.h ipc.h
	$(CC) $(CCFLAGS) -c client.c

admin1.o: admin1.c ipc.h utils_v1.h
	$(CC) $(CCFLAGS) -c admin1.c 

utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c 

network.o: network.c network.h
	$(CC) $(CCFLAGS) -c network.c 

game.o: game.c game.h ipc.h
	$(CC) $(CCFLAGS) -c game.c

ipc.o: ipc.c ipc.h
	$(CC) $(CCFLAGS) -c ipc.c 

clear :
		clear

clean :
	rm -f *.o
	rm -f server
	rm -f client

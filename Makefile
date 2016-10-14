CC = g++
BIN = server client

.PHONY: all clean

all: $(BIN)

build: server client


server: server.o
	$(CC) -lnsl -std=c++11 server.o -o server

server.o: server.cpp
	$(CC) -lnsl -std=c++11 -c server.cpp

client: client.o
	$(CC) -lnsl -std=c++11 client.o -o client

client.o: client.cpp
	$(CC) -lnsl -std=c++11 -c client.cpp

clean:
	rm -f *.o *~
	rm -f server client

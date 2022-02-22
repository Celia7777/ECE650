CCFLAGS=--std=gnu++98 -pedantic -Wall -Werror -ggdb3 -std=c++11

all: ringmaster player

ringmaster:  ringmaster.o socket.o
	g++ -o ringmaster $(CCFLAGS)  ringmaster.o socket.o

player:  player.o socket.o
	g++ -o player $(CCFLAGS)  player.o socket.o

%.o: %.cpp potato.h socket.h
	g++ -c $(CCFLAGS) $<

clean:
	rm -f *.o  *~ ringmaster player
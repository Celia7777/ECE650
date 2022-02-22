all: ringmaster player

ringmaster: ringmaster.cpp socket.h potato.h
	g++ -g -o ringmaster ringmaster.cpp
player: player.cpp socket.h potato.h
	g++ -g -o player player.cpp
clean:
	rm -rf ringmaster player
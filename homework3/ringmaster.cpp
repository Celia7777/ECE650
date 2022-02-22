#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "socket.h"


/* ringmaster create TCP connections to players */
void communicatePlayers(int players_num, int ringmaster_fd, std::vector<int> &players_fd, std::vector<std::string> &players_ip, std::vector<int> &players_port){
    for(int id = 0; id < players_num; id++){
        std::string ip;
        int port;
        //player accept the socket connection
        int connect_to_player_fd = toAccept(ringmaster_fd, ip);
        //ringmaster assign id to players
        send(connect_to_player_fd, &id, sizeof(id), 0);
        //ringmaster tell each player the total number of players
        send(connect_to_player_fd, &players_num, sizeof(players_num), 0);
        //ringmaster receive the port number of each player
        recv(connect_to_player_fd, &port, sizeof(port), 0);

        //update the players info
        players_fd.push_back(connect_to_player_fd);
        players_ip.push_back(ip);
        players_port.push_back(port);
        //upon connection with player, each player should send some initial msg to ringmaster
        std::cout<<"Player "<<id<<" is ready to play"<<std::endl;

    }
}

void createRoutinginfo(int players_num, std::vector<int> &players_fd, std::vector<std::string> &players_ip, std::vector<int> &players_port){
    for(int id = 0; id < players_num; id++){
        //remember it is a circle
        //initialize the neighbor info
        int neighbor_id = (id + 1) % players_num;
        int neighbor_port = players_port[neighbor_id];
        char ip[50];
        memset(ip, 0, sizeof(ip)); //make sure the char is empty
        strcpy(ip, players_ip[neighbor_id].c_str());
        //ringmaster send the neighbor info to the current player for routing
        send(players_fd[id], &neighbor_port, sizeof(neighbor_port), 0);
        send(players_fd[id], &ip, sizeof(ip), 0);
    }
}

int main(int argc, char *argv[]){
    if(argc != 4){
        std::cerr<<"ringmaster <port_num> <num_players> <num_hops>"<<std::endl;
        return EXIT_FAILURE;
    }

    // get the variables from the commands
    const char* server_port = argv[1];
    int players_num = atoi(argv[2]);
    int hops_num = atoi(argv[3]);
    
    //check the correctness of the commands
    if(players_num <= 1){
        std::cerr<<"<num_players> must be greater than 1"<<std::endl;
        return EXIT_FAILURE;
    }
    if(hops_num < 0 || hops_num > 512){
        std::cerr<<"<num_hops> must be greater than or equal to zero and less than or equal to 512"<<std::endl;
        return EXIT_FAILURE;
    }

    //print the initial messages from ringmaster
    std::cout<<"Potato Ringmaster"<<std::endl;
    std::cout<<"Players = "<<players_num<<std::endl;
    std::cout<<"Hops = "<<hops_num<<std::endl;

    //create the ringmaster socket
    int ringmaster_fd = setupRingmaster(server_port);
    //store players info
    std::vector<int> connect_to_players_fd;
    std::vector<std::string> players_ip;
    std::vector<int> players_port;

    //ringmaster start communicating with players
    communicatePlayers(players_num, ringmaster_fd, connect_to_players_fd, players_ip, players_port);
    //ringmaster tell the current player the routing info
    createRoutinginfo(players_num, connect_to_players_fd, players_ip, players_port);

    //start passing potato
    Potato potato;
    potato.hops_num = hops_num;
    //ringmaster sends the potato to a player randomly
    if(hops_num != 0){
        srand((unsigned int)time(NULL) + players_num);
        int random = rand() % players_num;
        send(connect_to_players_fd[random], &potato, sizeof(potato), 0);
        //When launching the potato to the first randomly chosen player:
        std::cout<<"Ready to start the game, sending potato to player " << random << std::endl;
        //ringmaster receives the "it"
        filefdSelect(connect_to_players_fd, potato);
    }

    //shuts the game down (by sending a message to each player 
    //to indicate they may shut down as the game is over
    //shut down signal
    // for (int id = 0; id < players_num; id++) {
    //     send(connect_to_players_fd[id], &potato, sizeof(potato), 0);
    // }
    std::cout << "Trace of potato:" << std::endl;
    potato.printPath();
    //shuts the game down
    //close sockets connecting to players
    for(int id = 0; id < players_num; id++){
        close(connect_to_players_fd[id]);
    }
    //close socket listening
    close(ringmaster_fd);

    return EXIT_SUCCESS;
    
}

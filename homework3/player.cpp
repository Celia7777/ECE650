#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "socket.h"

int main(int argc, char *argv[]){
    if(argc != 3){
        std::cerr<<"player <machine_name> <port_num>"<< std::endl;
        return EXIT_FAILURE;
    }

    // get the variables from the commands
    const char *domain = argv[1];
    const char *port = argv[2];
    int players_num;
    int id;
    int neighbor_port;
    char neighbor_ip[50];
    std::string neighborip_fromplayer;

    //create tcp connection to ringmaster
    int player_as_client_fd = setupPlayer(domain, port);
    //player receives id from ringmaster
    recv(player_as_client_fd, &id, sizeof(id), 0);
    recv(player_as_client_fd, &players_num, sizeof(players_num), 0);

    //After receiving an initial message from the ringmaster to tell the player 
    //the total number of players in the game, and possibly other information 
    //(e.g. info about that player’s neighbors， id)
    std::cout<<"Connected as player "<<id<<" out of "<<players_num<<" total players"<<std::endl;

    //player becomes as server, send its port number to ringmaster
    int player_as_server_fd = setupRingmaster("");
    int player_port = getPort(player_as_server_fd);
    send(player_as_client_fd, &player_port, sizeof(player_port), 0);

    //player receives the routing info (neighbors' ip, port number)
    recv(player_as_client_fd, &neighbor_port, sizeof(neighbor_port), MSG_WAITALL);
    recv(player_as_client_fd, &neighbor_ip, sizeof(neighbor_ip), MSG_WAITALL);

    //player communicate with its neighbor
    //setup sockect for neighbor, need to convert the int(port) to char(port)
    char neighbor_port_char[9];
    sprintf(neighbor_port_char, "%d", neighbor_port);
    //initialize the left and right neighbors
    int neighbor_fd_right = setupPlayer(neighbor_ip, neighbor_port_char);
    int neighbor_fd_left = toAccept(player_as_server_fd, neighborip_fromplayer);
    int right_id = (id + 1) % players_num;
    int left_id = (id - 1 + players_num) % players_num;


    //start passing potato
    Potato potato;
    std::vector<int> routing_fds;
    routing_fds.push_back(player_as_client_fd);
    routing_fds.push_back(neighbor_fd_right);
    routing_fds.push_back(neighbor_fd_left);
    
    //int i = 1;
    while(1){
        //std::cout<<"in while "<<i<<std::endl;
        int len = filefdSelect(routing_fds, potato);
        //std::cout << "length is: "<<len<<std::endl;
        //shut down
        if(len == 0){
            break;
        }
        else if(potato.hops_num > 0){
            //handle the last potato, send to ringmaster
            if(potato.hops_num == 1){
                potato.hops_num--;
                potato.path[potato.path_idx] = id;
                potato.path_idx++;
                send(player_as_client_fd, &potato, sizeof(potato), 0);
                //When number of hops is reached:
                std::cout<<"I'm it"<<std::endl;
                //break;
            }
            //randomly choose the left or right neighbor
            else{
                potato.hops_num--;
                potato.path[potato.path_idx] = id;
                potato.path_idx++;
                srand((unsigned int)time(NULL)+potato.path_idx);
                int random = rand() % 2;
                if (random == 0) {
                    send(neighbor_fd_left, &potato, sizeof(potato), 0);
                    std::cout << "Sending potato to " << left_id << std::endl;
                }
                else{
                    send(neighbor_fd_right, &potato, sizeof(potato), 0);
                    std::cout << "Sending potato to " << right_id << std::endl;
                }
            }
        }
        //i++;
        

    }

    //close sockets
    for(size_t i = 0; i < routing_fds.size(); i++){
        close(routing_fds[i]);
    }

    return EXIT_SUCCESS;

}
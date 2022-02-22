#include "socket.h"


int setupRingmaster(const char * port){
        int socket_fd;//socket file destripter
        struct addrinfo hints, *servinfo;
        memset(&hints, 0, sizeof(hints));//make sure sturct is empty
        hints.ai_family = AF_UNSPEC; //use IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM;//use tcp
        hints.ai_flags = AI_PASSIVE;//assign my local host to socket

            //getaddrinfo, hostname, service
            if (getaddrinfo(NULL, port, &hints, &servinfo)!=0){
                //Given node and service, use getaddrinfo to get the internet address
                perror("Fail to get address in ringmaster");
                return -1;
            }

            //loop over the address info result and bind to the first as we can
            struct addrinfo *temp;
            int opt = 1;
            for(temp = servinfo; temp!=NULL; temp = temp->ai_next){
                //socket creation
                if((socket_fd=socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol))==-1){
                    perror("Fail to create socket");
                    continue;
                }
                //socket set up, in case socket use the same address
                if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
                    perror("Fail to set socket");
                    return -1;
                } 
                //socket bind
                if (bind(socket_fd, temp->ai_addr, temp->ai_addrlen) == -1) { 
                    close(socket_fd);
                    perror("Fail to bind socket");
                    continue; 
                }
                break;
            }
            freeaddrinfo(servinfo);
            if (listen(socket_fd, 100) == -1) { 
                perror("Fail to listen");
                return -1;
            }
            // }else{
            //     std::cout<< "Starting listening" << std::endl;
            // }
            return socket_fd;
}


int toAccept(int socket_fd, std::string &ip){
    // while(fileSelect(socket_fd)!=1){
        
    // }
    struct sockaddr_storage connect_addr;//fill in with the address of the site which is trying to connect to the server
    socklen_t socket_addr_len = sizeof(connect_addr);//filled in with the sizeof() the structure returned in the addr parameter
    int accept_fd;
    accept_fd = accept(socket_fd, (struct sockaddr *)&connect_addr, &socket_addr_len);
    if(accept_fd==-1){
        perror("Fail to accept");
        return -1;
    }
    struct sockaddr_in * addr = (struct sockaddr_in *)&connect_addr;
    //std::cout << "accepting ip address: "<<inet_ntoa(addr->sin_addr) << std::endl;
    ip = inet_ntoa(addr->sin_addr);
  
    return accept_fd;
}

int filefdSelect(std::vector<int> &socket_fds, Potato &p){
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    for(size_t id = 0; id < socket_fds.size(); id++){
        FD_SET(socket_fds[id], &readfds);
    }
    int n = *(max_element(socket_fds.begin(), socket_fds.end())) + 1;

    tv.tv_sec = 10;
    tv.tv_usec = 600000;
    int rv = select(n, &readfds, NULL, NULL, &tv);
    int len;
    for(size_t id = 0; id < socket_fds.size(); id++){
        if(rv==-1){
            perror("error in select"); // error occurred in select()
        }
        else if(rv==0){
            std::cout<<"Timeout occurred! No data after 10.6 seconds."<<std::endl;
            return -1;
        } 
        else{
            if(FD_ISSET(socket_fds[id], &readfds)){
            len = recv(socket_fds[id], &p, sizeof(p), MSG_WAITALL);//0
            return len;
            }
        }
    }   
    return -1;
}

int setupPlayer(const char* domain, const char * port){
            int socket_fd;
            struct addrinfo hints, *servinfo, *temp;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            if (getaddrinfo(domain, port, &hints, &servinfo)!=0){
                //Given node and service, use getaddrinfo to get the internet address
                perror(domain);
                return -1;
            }

            for(temp = servinfo; temp != NULL; temp = temp->ai_next) {
                if ((socket_fd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1) {
                    perror("error in seting up player socket");
                    continue;
                }

                if (connect(socket_fd, temp->ai_addr, temp->ai_addrlen) == -1) {
                    close(socket_fd);
                    perror("error in connecting socket");
                    continue;
                }

                break;
            }

            //std::cout << "Client: Connect to server successfully\n";
            freeaddrinfo(servinfo);
            return socket_fd;
        }


int getPort(int socket_fd){
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(socket_fd, (struct sockaddr *)&addr, &addr_len) == -1) {
        perror("Error in getsockname");
        // std::cerr << "Error: cannot getsockname" << std::endl;
        // exit(EXIT_FAILURE);
    }
    return ntohs(addr.sin_port);
}

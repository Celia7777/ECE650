#ifndef SOCKET
#define SOCKET

#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>

#include "potato.h"

int setupRingmaster(const char * port);
int setupPlayer(const char* domain, const char * port);
int toAccept(int socket_fd, std::string &ip);
int filefdSelect(std::vector<int> &socket_fds, Potato &p);
int getPort(int socket_fd);



#endif
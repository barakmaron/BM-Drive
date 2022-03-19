#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#ifndef CLIENT_H
#define CLIENT_H

#define PORT 2477

struct Client
{
    int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr;
    int in_use;
};

typedef struct Client* CLIENT;

void ClientConstruct(CLIENT client, char* host_name);
int StartConnectionByProtocol(CLIENT client);
void HandleCommandsFromUser(CLIENT client);
void PrintListCommands();

#endif
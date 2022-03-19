#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <mysql/mysql.h>
#include "ConnectionNode.h"
#include "ProtocolDefinitions.h"
#include "CommunicationProtocol.h"
#include "DataBaseManager.h"

#define PORT 2477
#define MAX_USERS 25

struct Server
{
    int sockfd;
    struct ConnectionNode* head_of_connections;
    struct sockaddr_in address;   
    MYSQL_MANAGER* db_manager;
};

typedef struct Server* SERVER;

void ServerConstruct(SERVER server);

void AcceptNewConnections(SERVER server);

struct ConnectionNode* GetConnection(struct ConnectionNode* head_of_connections, int new_connection);

struct ConnectionNode* SafeInsertNewConnection(struct ConnectionNode* head_of_connections, int new_connection);

void SetProccessToConnection(struct ConnectionNode* connection_node, MYSQL_MANAGER* db_manager);

char* GetClientIp(int clientfd);

void HandleConnection(int connectionfd, MYSQL_MANAGER* db_manager);

void HandleMessage(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
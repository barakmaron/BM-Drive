#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <mysql/mysql.h>
#include "Server.h"
#include "ConnectionNode.h"
#include "ProtocolDefinitions.h"
#include "CommunicationProtocol.h"
#include "DataBaseManager.h"
#include "ProtocolParser.h"

void ServerConstruct(SERVER server)
{
    int opt = 1;
    server->head_of_connections = NULL;
    server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(server->sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    server->address.sin_family = AF_INET;
    server->address.sin_addr.s_addr = INADDR_ANY;
    server->address.sin_port = htons(PORT);
    if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }   
    if (bind(server->sockfd, (struct sockaddr *)&server->address, sizeof(server->address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server->sockfd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is up and ready!\n");
    printf("Listening on port: %d\n", PORT);

    server->db_manager = ConnectToDataBase();
    SetUpDataBase(server->db_manager);

    printf("Wating for new connections...\n");

    AcceptNewConnections(server);

    close(server->sockfd);
    FreeConnectionNode(server->head_of_connections);
}

void AcceptNewConnections(SERVER server)
{
    int new_connection, child;
    int addr_len = sizeof(server->sockfd);
    for(int i = 0;i < 250; i++)
    {
        new_connection = accept(server->sockfd, (struct soccaddr *)&server->address, (socklen_t*)&addr_len);
        if(new_connection < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        server->head_of_connections = SafeInsertNewConnection(server->head_of_connections, new_connection);
        GetClientIp(new_connection);
        SetProccessToConnection(GetConnection(server->head_of_connections, new_connection), server->db_manager);        
    }
}

struct ConnectionNode* GetConnection(struct ConnectionNode* head_of_connections, int new_connection)
{
    return FindConnectionNode(head_of_connections, new_connection);
}

struct ConnectionNode* SafeInsertNewConnection(struct ConnectionNode* head_of_connections, int new_connection)
{
    struct ConnectionNode* ret;
    if(!head_of_connections)
    {
        ret = malloc(sizeof(struct ConnectionNode));
        ret->next = NULL;
        ret->value = new_connection;
        return ret;
    }
    else    
        InsertConnectionNode(head_of_connections, new_connection);
    return head_of_connections;
}

void SetProccessToConnection(struct ConnectionNode* connection_node, MYSQL_MANAGER* db_manager)
{
    int child = fork();
    if(child == 0)
        HandleConnection(connection_node->value, db_manager);
}

char* GetClientIp(int clientfd)
{
    struct sockaddr_in addr;
    int len_addr = sizeof(addr);
    if(getsockname(clientfd, &addr, &len_addr) < -1)
    {
        perror("Get client name\n");
    }
    printf("Client IP address is: %s\n", inet_ntoa(addr.sin_addr));
}

void HandleConnection(int connectionfd, MYSQL_MANAGER* db_manager)
{
    MESSAGE msg_from_client;
    do
    {
        msg_from_client = ReadMessage(connectionfd);
        HandleMessage(msg_from_client, connectionfd, db_manager);
        FreeMessage(msg_from_client);
    } while (1); // ping socket to clear connection
    
}

void HandleMessage(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    switch (msg->code)
    {
    case START_CONNECTION:
        SendConnectionStarted(clientfd);
    break;
    case REGISTER_USER:
        RegisterNewUser(msg, clientfd, db_manager);
    break;
    case LOGIN_USER:
        LoginUser(msg, clientfd, db_manager);
    break;
    case CHANGE_DIRECTORY:
        ChangeDirectory(msg, clientfd, db_manager);
    break;
    case UPLOAD_ALL_FILES:
        UploadAllFiles(msg, clientfd, db_manager);
    break;
    case UPLOAD_FILE:
        UploadFile(msg, clientfd, db_manager);
    break;
    case UPLOAD_DIRECTORY:
        UploadDirectory(msg, clientfd, db_manager);
    break;
    case LOGOUT_USER:
        LogoutUser(msg, clientfd, db_manager);
    break;
    case SET_AUTO_SYNC:
        AutoSync(msg, clientfd, db_manager);
    break;
    case STOP_AUTO_SYNC:
        StopAutoSync(msg, clientfd);
    break;
    case SHARE_FILE:
        ShareFileWithUser(msg, clientfd, db_manager);
    break;
    case GET_SHARED_FILES:
        GetSharedFiles(msg, clientfd, db_manager);
    break;
    default:
    break;
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include "Client.h"
#include "CommunicationProtocol.h"
#include "MessageHandle.h"
#include "ProtocolParser.h"


void ClientConstruct(CLIENT client, char* host_name)
{
    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(client->sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if(!host_name)
    {
        perror("No host name");
        exit(EXIT_FAILURE);
    }
    client->he  = gethostbyname(host_name);
    if(!client->he)
    {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    client->their_addr.sin_family = AF_INET;
    client->their_addr.sin_port = htons(PORT);
    client->their_addr.sin_addr = *((struct in_addr*)client->he->h_addr);
    bzero(&(client->their_addr.sin_zero), 8);

    if(connect(client->sockfd, (struct sockaddr *)&client->their_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

int StartConnectionByProtocol(CLIENT client)
{
    MESSAGE msg_form_server;
    MESSAGE msg_for_server;

    msg_for_server = Message(START_CONNECTION, NULL, NULL);
    SendMessage(msg_for_server, client->sockfd);

    msg_form_server = ReadMessage(client->sockfd);

    switch (msg_form_server->code)
    {
    case CONNECTION_STARTED:
        FreeMessage(msg_form_server);
        FreeMessage(msg_for_server);
        return 1;
        break;
    
    case CONNECTION_FAILED:
        FreeMessage(msg_form_server);
        FreeMessage(msg_for_server);
        return 0;
        break;
    }    
    return 0;
}

void HandleCommandsFromUser(CLIENT client)
{
    MESSAGE msg_sent_to_server = NULL;
    TOKEN token = NULL;
    char buffer;   
    printf("Enter command to execute to server: \n");
    scanf(" %c", &buffer); 
    getchar();
    while (buffer != 'q')
    {        
        switch(buffer)
        {
            case 'h':
                PrintListCommands();
            break;
            case 'r':
                msg_sent_to_server = RegisterUser(client->sockfd);               
            break;
            case 'l':
                msg_sent_to_server = LoginUser(client->sockfd);                
            break;
            case 'd':
                msg_sent_to_server = SetDirectory(client->sockfd, token);                
            break;
            case 'u':
                msg_sent_to_server = UploadAllFiles(client->sockfd, token);                
            break;
            case 'o':
                msg_sent_to_server = LogoutUser(client->sockfd, token);
            break;
            case 'f':
                msg_sent_to_server = UploadFile(client->sockfd, token);
            break;
            case 'a':
                msg_sent_to_server = SetAutoSync(client->sockfd, token);                
            break;
            case 't':
                msg_sent_to_server = StopAutoSync(client->sockfd, token);
            break;
            case 's':
                msg_sent_to_server = ShareFileWithOtherUser(client->sockfd, token);
            break;
            default:
                printf("Command not fund, please try 'h' to find out all of the valid commands!\n");
                msg_sent_to_server = NULL;
            break;
        }    
        if(msg_sent_to_server)
        {
            HandleMessageFromServer(client, msg_sent_to_server, &token);            
            FreeMessage(msg_sent_to_server);
        }        
        printf("Enter command to execute to server: \n");
        scanf(" %c", &buffer);
        getchar();
        msg_sent_to_server = NULL;
    }
    FreeToken(token);
    printf("By By...\n");// TODO: Send close msg to server
}


void PrintListCommands()
{
    system("clear");
    printf("------------------------START HELP------------------------\n");
    printf("This is the list of command that you can use:\n");
    printf("r\t Register new user\n");
    printf("l\t Login user\n");
    printf("d\t Set directory to share with sever\n");
    printf("f\t Upload only one file\n");
    printf("u\t Upload all file in directory\n");
    printf("a\t Auto sync files from directory as thay change\n");
    printf("t\t Stop auto sync\n");
    printf("s\t Share file with other user\n");
    printf("d\t Delete file from server\n");
    printf("o\t Logout user\n");
    printf("q\t To exit the program safely\n");
    printf("----------------------------END----------------------------\n");
}
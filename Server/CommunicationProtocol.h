#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "ProtocolDefinitions.h"
#include "Token.h"

#ifndef COMMUNICARIONPROTOCOL_H
#define COMMUNICARIONPROTOCOL_H

struct Message
{
    int num_of_arguments;
    int code;
    char** vars;
    TOKEN connection_token;
};

typedef struct Message* MESSAGE;

MESSAGE MessageFromString(char* message_from_client);

MESSAGE Message(int protocol_code, char* vars, TOKEN token);

MESSAGE ReadMessage(int clientfd);

void ParseArguments(MESSAGE msg, char* message);

char* ParseTwoArgToString(char* arg1, char* arg2);

int SendMessage(MESSAGE msg, int clientfd);

int SendFile(int file, int clientfd, struct stat file_stat);

char* ReadMsgBitByBitFromSocket(int clientfd);

int RecevAndWriteFile(FILE* file, int file_size, int clientfd);

char* ToString(MESSAGE msg);

void FreeMessage(MESSAGE msg);

void PrintError(const char* msg);

#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "CommunicationProtocol.h"
#include "ProtocolDefinitions.h"
#include "Token.h"

MESSAGE Message(int protocol_code, char* vars, TOKEN token)
{
    MESSAGE ret;

    ret = malloc(sizeof(struct Message));
    ret->code = protocol_code;
    ParseArguments(ret, vars);
    ret->connection_token = CreateCopyToken(token);

    return ret;
}

MESSAGE MessageFromString(char* message_from_client)
{
    MESSAGE ret;
    char *temp_for_free_only = message_from_client;
    char *message_with_out_code;
    char code_in_string[] = "000\0";
    char *arg;
    int len = strlen(message_from_client), new_len;
    char *start_pos;
    char *end_pos;
    ret = malloc(sizeof(struct Message));
    memcpy(code_in_string, message_from_client, SIZE_OF_PROTOCOL_CODE_IN_STRING);
    ret->code = atoi(code_in_string);

    new_len = len - SIZE_OF_PROTOCOL_CODE_IN_STRING - SIZE_OF_DELIMITER + SIZE_OF_END_STRING;
    message_with_out_code = malloc(new_len);
    //Cut string after getting the Protocol code
    memcpy(message_with_out_code, (message_from_client + SIZE_OF_PROTOCOL_CODE_IN_STRING + SIZE_OF_DELIMITER), new_len);
    message_with_out_code[new_len  - 1] = '\0';

    start_pos = message_with_out_code;
    end_pos = start_pos ? strchr(start_pos + 1, '|') - 1: NULL;
    if(end_pos && start_pos && start_pos[0] != '*')
    {
        new_len = end_pos - start_pos + SIZE_OF_END_STRING;
        arg = malloc(new_len+ SIZE_OF_END_STRING);
        strncpy(arg, start_pos, new_len);
        arg[new_len] = '\0';
        ParseArguments(ret, arg);
        free(arg);
    }
    else
        ParseArguments(ret, NULL);
    start_pos = strchr(message_with_out_code, '|') + SIZE_OF_DELIMITER;
    if(message_from_client[0] != '\0' && start_pos[0] != '*')
    {
        end_pos = strchr(message_from_client, '\0');
        if(start_pos && end_pos && start_pos[0] != '\0')
        {
            //memcpy(message_from_client, start_pos,  end_pos - start_pos);
            ret->connection_token = CreateTokenFromString(start_pos);
        }
        else 
            ret->connection_token = NULL;
    }
    else 
        ret->connection_token = NULL;
    free(temp_for_free_only);
    free(message_with_out_code);
    return ret;
}

MESSAGE ReadMessage(int clientfd)
{
    return MessageFromString(ReadMsgBitByBitFromSocket(clientfd));
}

char* ReadMsgBitByBitFromSocket(int clientfd)
{
    char buffer[1];
    char msg[1024] = "";
    char* ret;
    int status; 
    int i = 0;
    do
    {
        bzero(buffer, 1);
        status = recv(clientfd, buffer, 1, 0);

        if(status == -1)
            PrintError(strerror(errno));
        else if(status == 0)
            PrintError("ERROR: empty recv from client.");
        msg[i] = buffer[0];
        i++;
    } while (*buffer != '\0'); 
    ret = malloc(i);
    strncpy(ret, msg, i - 1);
    ret[i - 1] = '\0';
    return ret;
}

int RecevAndWriteFile(FILE* file, int file_size, int clientfd)
{
    char buffer[BUFSIZ];
    int len;
    int remain_data = 0;
    remain_data = file_size;
    while ((remain_data > 0) && ((len = recv(clientfd, buffer, BUFSIZ, 0)) > 0))
    {
        fwrite(buffer, sizeof(char), len, file);
        remain_data -= len;
        fprintf(stdout, "Receive %d bytes and we hope : %d bytes\n", len, remain_data);
    }
    return remain_data;
}

int SendMessage(MESSAGE msg, int clientfd)
{
    int status;
    char* parssed_msg = ToString(msg);
    status = send(clientfd, parssed_msg, strlen(parssed_msg) + SIZE_OF_END_STRING, 0);
    free(parssed_msg);
    if(status == -1)
    {
        PrintError(strerror(errno));
        return 0;
    }
    return 1;
}

int SendFile(int file, int clientfd, struct stat file_stat)
{
    ssize_t sent;
    off_t offset = 0;
    offset = 0;
    for (size_t size_to_send = file_stat.st_size; size_to_send > 0; )
    {
        sent = sendfile(clientfd, file, &offset, size_to_send);

        if (sent <= 0)
        {
            // Error or end of file
            if (sent != 0)
                perror("sendfile");  // Was an error, report it
            break;
        }

        size_to_send -= sent;  // Decrease the length to send by the amount actually sent
    }
    return offset;
}

char* ToString(MESSAGE msg)
{
    char code_in_string[SIZE_OF_PROTOCOL_CODE_IN_STRING + SIZE_OF_END_STRING];
    int length_of_string, length_of_token;
    char* ret_of_function;
    char *token_string;
    int size_of_vars = 0; 
    if(msg->num_of_arguments)   
    {
        for(int i = 0;i < msg->num_of_arguments;i++)
        {
            size_of_vars += strlen(msg->vars[i]);
            size_of_vars += SIZE_OF_DELIMITER;
        }
    }
    else
        size_of_vars = 1;
    token_string = ToStringToken(msg->connection_token);
    length_of_token = token_string ? SIZE_OF_DELIMITER + strlen(token_string) : 1;
    length_of_string = SIZE_OF_PROTOCOL_CODE_IN_STRING + SIZE_OF_DELIMITER + size_of_vars + SIZE_OF_DELIMITER + length_of_token + SIZE_OF_END_STRING;
    ret_of_function = malloc(length_of_string);
    sprintf(code_in_string, "%d", msg->code);
    code_in_string[3] = '\0';
    strncpy(ret_of_function, code_in_string, SIZE_OF_PROTOCOL_CODE_IN_STRING + SIZE_OF_END_STRING);
    strcat(ret_of_function, "|");

    if(msg->num_of_arguments)   
    {
        for(int i = 0;i < msg->num_of_arguments;i++)
        {
            strncat(ret_of_function, msg->vars[i], strlen(msg->vars[i]));
            if(i + 1 < msg->num_of_arguments)
                strcat(ret_of_function, "&");
        }
    }
    else
       strcat(ret_of_function, "*");
    if(length_of_token != 1)
    {
        strcat(ret_of_function, "|");
        strncat(ret_of_function, token_string, strlen(token_string));
    }
    else
        strcat(ret_of_function, "|*");
    strcat(ret_of_function, "\0");
    free(token_string);
    return ret_of_function;
}

void ParseArguments(MESSAGE msg, char* message)
{
    int num = 1;
    char *temp, **vars;
    char *arg_start_postion, *arg_end_postion, *delimiter_pos;
    int len, new_len;
    if(message != NULL && message[0] != '\0')
    {
        len = strlen(message);
        temp = malloc(len + SIZE_OF_END_STRING);
        strncpy(temp, message, len);
        temp[len] = '\0';

        delimiter_pos = strchr(temp, '&');
        while(delimiter_pos)
        {
            new_len = len - (delimiter_pos - temp);
            strncpy(temp, delimiter_pos + SIZE_OF_DELIMITER, new_len);
            delimiter_pos = strchr(temp, '&');
            num++;
        }
        msg->num_of_arguments = num;

        strncpy(temp, message, len);
        temp[len] = '\0';
        vars = malloc(sizeof(char *) * num);

        for(int i = 0;i < num;i++)
        {
            arg_start_postion = temp;
            arg_end_postion = strchr(temp, '&') ? strchr(temp, '&') : strchr(temp, '\0');
            new_len = arg_end_postion - arg_start_postion + SIZE_OF_END_STRING;
            vars[i] = malloc(new_len);
            strncpy(vars[i], arg_start_postion, new_len - SIZE_OF_END_STRING);
            vars[i][new_len - SIZE_OF_END_STRING] = '\0';
            new_len = strchr(temp, '\0') - (arg_end_postion + SIZE_OF_DELIMITER) + SIZE_OF_END_STRING;
            memcpy(temp, arg_end_postion + SIZE_OF_DELIMITER, new_len);
        }

        msg->vars = vars;
        free(temp);
    }
    else
    {
        msg->vars = NULL;
        msg->num_of_arguments = 0;
    }
}


char* ParseTwoArgToString(char* arg1, char* arg2)
{
    int argv_len = strlen(arg2) + SIZE_OF_DELIMITER + strlen(arg1) + SIZE_OF_END_STRING;
    char *argv;
    argv = malloc(argv_len);
    bzero(argv, argv_len);
    strcat(argv, arg1);
    strcat(argv, "&");
    strcat(argv, arg2);
    strcat(argv, "\0");
    return argv;
}

void FreeMessage(MESSAGE msg)
{
    if(msg->vars)
    {
        for(int i = 0;i < msg->num_of_arguments;i++)
        {
            free(msg->vars[i]);
        }
        free(msg->vars);
    }
    FreeToken(msg->connection_token);
    free(msg);
    msg = NULL;
}

void PrintError(const char* msg)
{
    perror(msg);
    exit(1);
}
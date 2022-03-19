#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>486
#include "ProtocolParser.h"
#include "CommunicationProtocol.h"
#include "ProtocolDefinitions.h"

MESSAGE RegisterUser(int clientfd)
{
    char email[1024], password[1024], *argv;
    int len_email, len_password, len_argv;
    MESSAGE msg_for_server;
    printf("Enter Email:\n");
    scanf(" %s", email);
    printf("Enter Password:\n");
    scanf(" %s", password);
    len_email = strlen(email);
    len_password = strlen(password);
    len_argv = len_email + SIZE_OF_DELIMITER + len_password + SIZE_OF_END_STRING;
    argv = malloc(len_argv);
    bzero(argv, len_argv);
    strncat(argv, email, len_email);
    strcat(argv, "&");
    strncat(argv, password, len_password);
    strcat(argv, "\0");
    msg_for_server = Message(REGISTER_USER, argv, NULL);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        free(argv);
        return NULL;
    }
    free(argv);
    return msg_for_server;
}

MESSAGE LoginUser(int clientfd)
{
    char email[1024], password[1024], *argv;
    int len_email, len_password, len_argv;
    MESSAGE msg_for_server;
    printf("Enter Email:\n");
    scanf(" %s", email);
    printf("Enter Password:\n");
    scanf(" %s", password);
    len_email = strlen(email);
    len_password = strlen(password);
    len_argv = len_email + SIZE_OF_DELIMITER + len_password + SIZE_OF_END_STRING;
    argv = malloc(len_argv);
    bzero(argv, len_argv);
    strncat(argv, email, len_email);
    strcat(argv, "&");
    strncat(argv, password, len_password);
    strcat(argv, "\0");
    msg_for_server = Message(LOGIN_USER, argv, NULL);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        free(argv);
        return NULL;
    }
    free(argv);
    return msg_for_server;
}

MESSAGE SetDirectory(int clientfd, TOKEN token)
{
    char dir[1024], *vars;
    int len_dir;
    MESSAGE msg_for_server;
    printf("Enter directory:\n");
    fgets(dir, 1024, stdin);
    if(strlen(dir) > 0 && dir[strlen(dir) - 1] == '\n')
        dir[strlen(dir) - 1] = '\0';//Clean \n form input
    len_dir = strlen(dir) + SIZE_OF_END_STRING;
    vars = malloc(len_dir);
    bzero(vars, len_dir);
    strncat(vars, dir, len_dir - SIZE_OF_END_STRING);
    strcat(vars, "\0");
    msg_for_server = Message(CHANGE_DIRECTORY, vars, token);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        free(vars);
        return NULL;
    }
    free(vars);
    return msg_for_server;
}

MESSAGE UploadAllFiles(int clientfd, TOKEN token)
{
    MESSAGE msg_for_server;
    msg_for_server = Message(UPLOAD_ALL_FILES, NULL, token);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        return NULL;
    }
    return msg_for_server;
}

MESSAGE LogoutUser(int clientfd, TOKEN token)
{
    MESSAGE msg_for_server = NULL;  
    msg_for_server = Message(LOGOUT_USER, NULL, token);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        return NULL;
    }
    return msg_for_server;
}

MESSAGE UploadFile(int clientfd, TOKEN token)
{
    MESSAGE msg_for_server;
    char file_path[1024], file_size[256], *argv;
    int len;
    int file;
    struct stat file_stat;
    char *file_full_path;
    printf("Enter File Path:\n"); // get path
    fgets(file_path, 1024, stdin);
    if(strlen(file_path) > 0 && file_path[strlen(file_path) - 1] == '\n')
        file_path[strlen(file_path) - 1] = '\0';//Clean \n form input
    len = strlen(file_path) + SIZE_OF_END_STRING;
    file_full_path = malloc(len);
    bzero(file_full_path, len);
    strncat(file_full_path, file_path, len - SIZE_OF_END_STRING);
    strcat(file_full_path, "\0");

    file = open(file_full_path, O_RDONLY);
    if(file == -1)
    {
        perror("File: ");
        return NULL;
    }
    else if(fstat(file, &file_stat) < 0)
    {
        perror("File stat: ");
        return NULL;
    }
    sprintf(file_size, "%d", file_stat.st_size);

    argv = ParseTwoArgToString(file_full_path, file_size);
    msg_for_server = Message(UPLOAD_FILE, argv, token);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        return NULL;
    }

    close(file);
    free(file_full_path);
    free(argv);

    return msg_for_server;
}

MESSAGE SetAutoSync(int clientfd, TOKEN token)
{
    MESSAGE msg_for_server;
    msg_for_server = Message(SET_AUTO_SYNC, NULL, token);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        return NULL;
    }
    return msg_for_server;
}

MESSAGE StopAutoSync(int clientfd, TOKEN token)
{
    MESSAGE msg_for_server;
    msg_for_server = Message(STOP_AUTO_SYNC, NULL, token);
    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        return NULL;
    }
    return msg_for_server;
}

MESSAGE ShareFileWithOtherUser(int clientfd, TOKEN token)
{
    MESSAGE msg_for_server;
    char buffer[1024], *argv;
    int len;
    char *file_full_path, *user_email;

    printf("Enter File Path:\n"); // get path
    fgets(buffer, 1024, stdin);
    if(strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0';//Clean \n form input
    len = strlen(buffer) + SIZE_OF_END_STRING;
    file_full_path = malloc(len);
    bzero(file_full_path, len);
    strncat(file_full_path, buffer, len - SIZE_OF_END_STRING);
    strcat(file_full_path, "\0");

    bzero(buffer, 1024);// clean buffer
    printf("Enter Email of user:\n");
    fgets(buffer, 1024, stdin);// get email
    if(strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0';//Clean \n form input
    len = strlen(buffer) + SIZE_OF_END_STRING;

    user_email = malloc(len);
    bzero(user_email, len);
    strncat(user_email, buffer, len - SIZE_OF_END_STRING);
    strcat(user_email, "\0");

    argv = ParseTwoArgToString(file_full_path, user_email);
    msg_for_server = Message(SHARE_FILE, argv, token);

    if(!SendMessage(msg_for_server, clientfd))
    {
        perror("Sending message to server");
        FreeMessage(msg_for_server);
        return NULL;
    }

    free(user_email);
    free(file_full_path);
    free(argv);

    return msg_for_server;
}
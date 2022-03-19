#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include "Client.h"
#include "CommunicationProtocol.h"
#include "ProtocolDefinitions.h"
#include "Token.h"
#include "MessageHandle.h"
#include "ManageFileUploading.h"
#include "ManageFileSharing.h"

void HandleMessageFromServer(CLIENT client, MESSAGE msg_sent_to_server, TOKEN *token)
{
    MESSAGE msg_from_server;
    client->in_use = 1;
    msg_from_server = ReadMessage(client->sockfd);
    switch (msg_sent_to_server->code)
    {
    case REGISTER_USER:
        RegisterHandle(msg_from_server, msg_sent_to_server);
        break;    
    case LOGIN_USER:
        *token = LoginHandle(client->sockfd, msg_from_server, msg_sent_to_server);
        break;  
    case CHANGE_DIRECTORY:
        ChangeDirectoryHandle(msg_from_server, msg_sent_to_server);
        break;  
    case UPLOAD_ALL_FILES:
        UploadAllFileHandle(client->sockfd, msg_from_server, msg_sent_to_server);
        break;
    case UPLOAD_FILE:
        UploadFileHandle(client->sockfd,  msg_from_server, msg_sent_to_server);
    break;
    case LOGOUT_USER:
        LogoutHandle(client, msg_from_server, token);
    break;
    case SET_AUTO_SYNC:
        SetAutoSyncHandle(client, msg_from_server);
    break;
    case STOP_AUTO_SYNC:
        StopAutoSyncHandle(client, msg_sent_to_server);
    break;
    case SHARE_FILE:
        ShareFileHandle(client->sockfd, msg_from_server);
    break;
    default:
        break;
    }
    FreeMessage(msg_from_server);
    client->in_use = 0;
}

void RegisterHandle(MESSAGE msg_from_server, MESSAGE msg_sent_to_server)
{
    switch (msg_from_server->code)
    {
    case REGISTER_SUCCESSFULLY:
        printf("User: %s was registered seccessfully!\n", msg_sent_to_server->vars[0]);
        break;
    case REGISTER_FAILED:
        printf("User: %s was not registered please try again later!\n", msg_sent_to_server->vars[0]);
        break;
    case REGISTER_FAILED_PASSWORD_NOT_LEGAL:
        printf("User: %s was not registered, The password is not legal or to short!\n", msg_sent_to_server->vars[0]);
        break;
    case REGISTER_FAILED_EMAIL_IN_USE:
        printf("User: %s was not registered, The email is already in use try diffrent email or try to logging!\n", msg_sent_to_server->vars[0]);
        break;
    case REGISTER_FAILED_EMAIL_NOT_LEGAL:
        printf("User: %s was not registered, The email is not legal!\n", msg_sent_to_server->vars[0]);
        break;    
    }
}

TOKEN LoginHandle(int serverfd, MESSAGE msg_from_server, MESSAGE msg_sent_to_server)
{
    TOKEN ret = NULL;
    switch (msg_from_server->code)
    {
    case LOGIN_USER_SUCCESSFULLY:        
        printf("User: %s logged in seccessfully!\n", msg_sent_to_server->vars[0]);
        ret = malloc(sizeof(struct Token));
        TokenCopyConstructor(ret, msg_from_server->connection_token);
        // Get files shared with this user
        GetFilesSharedWithUser(serverfd, ret);
        break;  
    case LOGIN_FAILED_WRONG_CREDENTIALS:
        printf("User: %s was not logged in, The password and email do not match!\n", msg_sent_to_server->vars[0]);
        break;
    case LOGIN_FAILED_USER_NOT_REGISTERED:
        printf("User: %s was not logged in, The user is not registered!\n", msg_sent_to_server->vars[0]);
        break;
    case LOGIN_FAILED_EMAIL_NOT_LEGAL:
        printf("User: %s was not logged in, The email is not not legal!\n", msg_sent_to_server->vars[0]);
        break;    
    }
    return ret;
}

void ChangeDirectoryHandle(MESSAGE msg_from_server, MESSAGE msg_sent_to_server)
{
    switch (msg_from_server->code)
    {
    case CHANGE_DIRECTORY_SUCCESSFULLY:
        printf("Directory: %s is set seccessfully!\n", msg_sent_to_server->vars[0]);
        break;
    case CHANGE_DIRECTORY_FAILED_NOT_VALID_DIRECTORY:
        printf("Directory: %s, Not valid directory!!!\n", msg_sent_to_server->vars[0]);
        break;
    case CHANGE_DIRECTORY_FAILED:
        printf("Directory: %s, Changing directory failed!!!\n", msg_sent_to_server->vars[0]);
        break; 
    case TOKEN_NOT_VALID:
        printf("Token is not valid!!! Try to login again to get new token!\n");
        break;    
    }
}

void UploadAllFileHandle(int serverfd, MESSAGE msg_from_server, MESSAGE msg_sent_to_server)
{
    switch (msg_from_server->code)
    {
    case UPLOAD_ALL_FILES_START:
        printf("Upload all file has started for directory: %s\n", msg_from_server->vars[0]);
        StartUploadingFiles(msg_from_server->vars[0], serverfd, msg_from_server->connection_token);
        break;
    case UPLOAD_ALL_FILES_FAILED_STORE_FILE:
        printf("Upload Failed!!\n");
        break;
    case GET_DIRECTORY_FAILED:
        printf("Cant get directory from server!!\n");
        break; 
    case TOKEN_NOT_VALID:
        printf("Token is not valid!!! Try to login again to get new token!\n");
        break;    
    }
}

void LogoutHandle(CLIENT client, MESSAGE msg_from_server, TOKEN *token)
{
     switch (msg_from_server->code)
    {
    case LOGOUT_USER_SECCESSFULLY:
        FreeToken(*token);
        *token = NULL;
        printf("Logout successfully!!\n");
        break;
    case LOGOUT_USER_FAILED:
        printf("Logout Failed!!\n");
        break;   
    case TOKEN_NOT_VALID:
        printf("Token is not valid!!! Try to login again to get new token!\n");
        break;    
    }
}

void UploadFileHandle(int serverfd, MESSAGE msg_from_server, MESSAGE msg_sent_to_server)
{
    switch (msg_from_server->code)
    {
    case CAN_START_UPLOAD_OF_FILE:
        printf("Upload file has started for file: %s\n", msg_sent_to_server->vars[0]);
        UploadFileToServerHandle(msg_from_server, msg_sent_to_server->vars[0], serverfd, msg_sent_to_server->connection_token);
        break;
    case UPLOAD_ALL_FILES_FAILED_STORE_FILE:
        printf("Upload Failed!!\n");
        break;
    case GET_DIRECTORY_FAILED:
        printf("Cant get directory from server!!\n");
        break; 
    case TOKEN_NOT_VALID:
        printf("Token is not valid!!! Try to login again to get new token!\n");
        break;    
    }
}

void SetAutoSyncHandle(CLIENT client, MESSAGE msg_from_server)
{
    switch (msg_from_server->code)
    {
    case SET_AUTO_SYNC_SUCCESSFULLY:
        printf("Auto sync started!!!\n");
        SetAutoSyncProccess(client, msg_from_server->vars[0], msg_from_server->connection_token);
        break;
    case SET_AUTO_SYNC_FAILED:
        printf("Auto sync Failed to start!!\n");
        break;
    case GET_DIRECTORY_FAILED:
        printf("Cant get directory from server!!\n");
        break; 
    case TOKEN_NOT_VALID:
        printf("Token is not valid!!! Try to login again to get new token!\n");
        break;    
    }
}

void StopAutoSyncHandle(int clientfd, MESSAGE msg_from_server)
{
    switch (msg_from_server->code)
    {
    case STOP_AUTO_SYNC_SUCCESSFULLY:
        printf("Auto sync stop!!!\n");
        StopAutoSyncProccess();
        break;
    case STOP_AUTO_SYNC_FAILED:
        printf("Auto sync Failed to stop!!\n");
        break;
    case TOKEN_NOT_VALID:
        printf("Token is not valid!!! Try to login again to get new token!\n");
        break;    
    }
}

void ShareFileHandle(int clientfd, MESSAGE msg_from_server)
{
    switch (msg_from_server->code)
    {
    case FILE_SHARE_SUCCESSFULLY:
        printf("The file was successfully shared!!!\n");
        break;
    case FILE_SHARE_FAILDED:
        printf("The file was not shared, please try again later!!!\n");
    break;
    case FILE_NOT_EXIST:
        printf("The file was not found on the server, please try other file or check the file path!!!\n");
    break;
    case TOKEN_NOT_VALID:
        printf("Token is not valid!!! Try to login again to get new token!\n");
    break;    
    }
}
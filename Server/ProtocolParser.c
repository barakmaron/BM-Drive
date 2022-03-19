#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <signal.h>
#include <mysql/mysql.h>
#include <unistd.h>
#include "ProtocolDefinitions.h"
#include "DataBaseManager.h"
#include "CommunicationProtocol.h"
#include "FileManager.h"

void SendConnectionStarted(int clientfd)
{
    MESSAGE msg_for_client = Message(CONNECTION_STARTED, NULL, NULL);
    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);
}

void RegisterNewUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    char *email, *password;
    MESSAGE msg_for_client;
    email = malloc(strlen(msg->vars[0]) + SIZE_OF_END_STRING);
    password = malloc(strlen(msg->vars[1]) + SIZE_OF_END_STRING);
    memcpy(email, msg->vars[0], strlen(msg->vars[0]) + SIZE_OF_END_STRING);
    memcpy(password, msg->vars[1], strlen(msg->vars[1]) + SIZE_OF_END_STRING);

    if(!strchr(email, '@') && !strchr(email, '.'))
        msg_for_client = Message(REGISTER_FAILED_EMAIL_NOT_LEGAL, NULL, NULL);
    else if(strlen(password) - 1 < 6)
        msg_for_client = Message(REGISTER_FAILED_PASSWORD_NOT_LEGAL, NULL, NULL);
    else
    {
        while(db_manager->in_use)
            sleep(0);// Wait for other child to finish with the data base
        db_manager->in_use = true;// Lock the data base for other children
        if(!CheckUserIsRegistered(db_manager, email))
        {
            if(!InsertNewUser(db_manager, email, password))
                msg_for_client = Message(REGISTER_SUCCESSFULLY, NULL, NULL);
            else
                msg_for_client = Message(REGISTER_FAILED, NULL, NULL);
        }
        else
            msg_for_client = Message(REGISTER_FAILED_EMAIL_IN_USE, NULL, NULL);
        db_manager->in_use = false;// Open lock the data base for other children
    }
    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);
    free(email);
    free(password);
}

void LoginUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    char *email, *password;
    MESSAGE msg_for_client;
    TOKEN token;
    email = malloc(strlen(msg->vars[0]) + SIZE_OF_END_STRING);
    password = malloc(strlen(msg->vars[1]) + SIZE_OF_END_STRING);
    memcpy(email, msg->vars[0], strlen(msg->vars[0]) + SIZE_OF_END_STRING);
    memcpy(password, msg->vars[1], strlen(msg->vars[1]) + SIZE_OF_END_STRING);

    if(!strchr(email, '@') && !strchr(email, '.'))
        msg_for_client = Message(LOGIN_FAILED_EMAIL_NOT_LEGAL, NULL, NULL);
    else if(strlen(password) - 1 < 6)
        msg_for_client = Message(LOGIN_FAILED_WRONG_CREDENTIALS, NULL, NULL);
    else
    {
        while(db_manager->in_use)
            sleep(0);// Wait for other child to finish with the data base
        db_manager->in_use = true;// Lock the data base for other children
        if(CheckUserIsRegistered(db_manager, email))
        {
            if(CheckUserCredentials(db_manager, email, password))
            {
                token = CreateToken(GetUserId(db_manager, email));
                msg_for_client = Message(LOGIN_USER_SUCCESSFULLY, NULL, token);
            }
            else
                msg_for_client = Message(LOGIN_FAILED_WRONG_CREDENTIALS, NULL, NULL);
        }
        else
            msg_for_client = Message(LOGIN_FAILED_USER_NOT_REGISTERED, NULL, NULL);
        db_manager->in_use = false;// Open lock the data base for other children
    }
    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);
    free(email);
    free(password);
}

void ChangeDirectory(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    char *dir;
    MESSAGE msg_for_client;
    dir = malloc(strlen(msg->vars[0]) + SIZE_OF_END_STRING);
    strncpy(dir, msg->vars[0], strlen(msg->vars[0])  + SIZE_OF_END_STRING);
    if(strchr(dir, '?') || strchr(dir, '\"') || strchr(dir, '<') || strchr(dir, '>') || strchr(dir, '*') || strchr(dir, '|'))
        msg_for_client = Message(CHANGE_DIRECTORY_FAILED_NOT_VALID_DIRECTORY, NULL, msg->connection_token);
    else
    {
        while(db_manager->in_use)
            sleep(0);// Wait for other child to finish with the data base
        db_manager->in_use = true;// Lock the data base for other children
        if(ValidateToken(msg->connection_token))
        {
            if(ChangeDirectoryInDataBase(db_manager, dir, msg->connection_token->token_string))
                msg_for_client = Message(CHANGE_DIRECTORY_SUCCESSFULLY, NULL, msg->connection_token);
            else
                msg_for_client = Message(CHANGE_DIRECTORY_FAILED, NULL, msg->connection_token);
        }
        else
            msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);
        db_manager->in_use = false;// Open lock the data base for other children
    }

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);
    free(dir);
}

void UploadAllFiles(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    char *dir;
    MESSAGE msg_for_client;

    if(ValidateToken(msg->connection_token))
    {
        dir = GetDirectoryFromDataBase(db_manager, msg->connection_token->token_string);
        if(dir)
            if(MakeDirectoryOnServerForUser(msg->connection_token->token_string) != -1)
                msg_for_client = Message(UPLOAD_ALL_FILES_START, dir, msg->connection_token);
            else
                msg_for_client = Message(UPLOAD_ALL_FILES_FAILED_STORE_FILE, dir, msg->connection_token);
        else
            msg_for_client = Message(GET_DIRECTORY_FAILED, NULL, msg->connection_token);
        free(dir);
    }
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);    
}

void UploadFile(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    char *dir, *dir_file;
    FILE* file;
    int file_size;
    MESSAGE msg_for_client;
        
    if(ValidateToken(msg->connection_token))
    {
        dir = GetDirectoryFromDataBase(db_manager, msg->connection_token->token_string);
        if(dir)
        {
            dir_file = CutDirectoryAndSetLocalDirectory(dir, msg->vars[0], msg->connection_token->token_string);
            file_size = atoi(msg->vars[1]);
            file = CreateFile(dir_file);            
            if(file)
            {
                msg_for_client = Message(CAN_START_UPLOAD_OF_FILE, NULL, msg->connection_token);
                SendMessage(msg_for_client, clientfd);
                if(RecevAndWriteFile(file, file_size, clientfd) == 0)
                {
                    FreeMessage(msg_for_client);
                    msg_for_client = Message(GOT_FILE_SUCCESSFULLY, NULL, msg->connection_token);
                } 
                else
                {
                    FreeMessage(msg_for_client);
                    msg_for_client = Message(FILE_UPLOAD_FAILED, NULL, msg->connection_token);
                }                   
                fclose(file);
            }
            else
                msg_for_client = Message(FAILED_TO_CREATE_FILE_ON_SERVER, NULL, msg->connection_token);
        }
        free(dir_file);
        free(dir);
    }
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);
    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client); 
}

void UploadDirectory(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    char *dir, *dir_file;
    MESSAGE msg_for_client;
        
    if(ValidateToken(msg->connection_token))
    {
        dir = GetDirectoryFromDataBase(db_manager, msg->connection_token->token_string);
        if(dir)
        {
            dir_file = CutDirectoryAndSetLocalDirectory(dir, msg->vars[0], msg->connection_token->token_string);
            if(MakeDirectory(dir_file) != -1)
            {
                msg_for_client = Message(MKDIR_SUCCESSFULLY, NULL, msg->connection_token);
            }
            else
                msg_for_client = Message(MKDIR_FAILED, NULL, msg->connection_token);            
        }
        free(dir_file);
        free(dir);
    }
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client); 
}

void LogoutUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    MESSAGE msg_for_client;
        
    if(ValidateToken(msg->connection_token))
        msg_for_client = Message(LOGOUT_USER_SECCESSFULLY, NULL, msg->connection_token);    
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client); 
}

void AutoSync(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    char *dir;
    MESSAGE msg_for_client;

    if(ValidateToken(msg->connection_token))
    {
        dir = GetDirectoryFromDataBase(db_manager, msg->connection_token->token_string);
        if(dir)
            msg_for_client = Message(SET_AUTO_SYNC_SUCCESSFULLY, dir, msg->connection_token);           
        else
            msg_for_client = Message(GET_DIRECTORY_FAILED, NULL, msg->connection_token);
        free(dir);
    }
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);    
}

void StopAutoSync(MESSAGE msg, int clientfd)
{
    MESSAGE msg_for_client;

    if(ValidateToken(msg->connection_token))
    {       
        msg_for_client = Message(STOP_AUTO_SYNC_SUCCESSFULLY, NULL, msg->connection_token);
    }
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);    
}

void ShareFileWithUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{

    MESSAGE msg_for_client;
    int is_shared_successfully;

    if(ValidateToken(msg->connection_token))
    {
        // Check if file exist in server
        if(CheckIfFileExistInServer(db_manager, msg->vars[0], msg->connection_token->token_string))
        {
            // Upload file to data base
            is_shared_successfully = SetFileToBeShared(db_manager, msg->vars[0], msg->vars[1], msg->connection_token->token_string);
            // Check if the file was saved in data base
            if(is_shared_successfully)
                msg_for_client = Message(FILE_SHARE_SUCCESSFULLY, NULL, msg->connection_token);
            else
                msg_for_client = Message(FILE_SHARE_FAILDED, NULL, msg->connection_token);
        }
        else
            msg_for_client = Message(FILE_NOT_EXIST, NULL, NULL);
    }
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);    
}

void GetSharedFiles(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager)
{
    MESSAGE msg_for_client, msg_from_client;
    int num_of_files_shared;
    char vars[4];
    char *argv;
    char **array_of_files_location = malloc(sizeof(char*));
    int file;
    char file_size[256];
    struct stat file_stat;
    if(ValidateToken(msg->connection_token))
    {
        //Get number of shared files
        num_of_files_shared = GetListOfSharedFiles(db_manager, msg->connection_token->token_string, &array_of_files_location);
        if(num_of_files_shared > 0)
        {
            sprintf(vars, "%d", num_of_files_shared);
            // Send number of files to client
            msg_for_client = Message(FOUND_SHARED_FILES, vars, NULL);
            if(!SendMessage(msg_for_client, clientfd))
            {
                perror("Sending message to server");
                FreeMessage(msg_for_client);
                for(int i = 0;i < num_of_files_shared;i++)
                    free(array_of_files_location[i]);
                free(array_of_files_location);
                return;
            }
            FreeMessage(msg_for_client);
            //Start sending files
            for(int i = 0; i < num_of_files_shared;i++)
            {
                //Open file and get file info for sending
                file = open(array_of_files_location[i], O_RDONLY);
                if(file == -1)
                    perror("File: ");
                else if(fstat(file, &file_stat) < 0)
                    perror("File stat: ");
                if(file != -1)
                {
                    sprintf(file_size, "%d", file_stat.st_size);
                    //Send client that files will be send know and the file name
                    argv =  ParseTwoArgToString(array_of_files_location[i], file_size);
                    msg_for_client = Message(START_UPLOAD_FILE, argv, NULL);
                    if(!SendMessage(msg_for_client, clientfd))
                    {
                        perror("Sending message to server");
                        FreeMessage(msg_for_client);
                        for(int i = 0;i < num_of_files_shared;i++)
                            free(array_of_files_location[i]);
                        free(array_of_files_location);
                        return;
                    }
                    FreeMessage(msg_for_client);
                    free(argv);
                    //Get OK from client to send file
                    msg_from_client = ReadMessage(clientfd);
                    if(msg_from_client->code == CAN_START_UPLOAD_OF_FILE)
                    {                    
                        SendFile(file, clientfd, file_stat);
                        //Get response from client that the file sent successfully
                        msg_from_client = ReadMessage(clientfd);
                        if(msg_from_client->code != GOT_FILE_SUCCESSFULLY)
                            perror("File upload went wrong!!");
                        else
                            printf("File: %s Was sent successfully!\n", array_of_files_location[i]);
                    }
                    else
                    {
                        perror("File upload went wrong!!");
                    }
                    FreeMessage(msg_from_client);
                    close(file);
                }
                else
                {
                    msg_for_client = Message(FILE_NOT_EXIST, NULL, NULL);
                    if(!SendMessage(msg_for_client, clientfd))
                    {
                        perror("Sending message to server");
                        FreeMessage(msg_for_client); 
                    }
                    FreeMessage(msg_for_client);
                }
            }
            // send to client finished message
            msg_for_client = Message(FINISHED_SHARED_FILES_DOWNLOAD, NULL, NULL);
        }
        else
            msg_for_client = Message(NO_SHARED_FILES, NULL, NULL);
    }
    else
        msg_for_client = Message(TOKEN_NOT_VALID, NULL, NULL);

    SendMessage(msg_for_client, clientfd);
    FreeMessage(msg_for_client);    
}
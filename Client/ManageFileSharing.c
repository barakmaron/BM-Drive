#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <signal.h>
#include "ManageFileSharing.h"
#include "ManageFileUploading.h"
#include "CommunicationProtocol.h"
#include "ProtocolDefinitions.h"
#include "Token.h"
#include "MessageHandle.h"

/*******************************************************************************
* INPUT: server socket, token of the connection;                               *
* OUTPUT: NULL;                                                                *
* Get the shared file from server and store theme in the client shared folder. *
*******************************************************************************/
void GetFilesSharedWithUser(int serverfd, TOKEN token)
{
    MESSAGE message_for_server, message_from_server;
    message_for_server = Message(GET_SHARED_FILES, NULL, token);
    if(!SendMessage(message_for_server, serverfd))
    {
        perror("Sending message to server");
        FreeMessage(message_for_server);
        return;
    }
    message_from_server = ReadMessage(serverfd);
    switch (message_from_server->code)
    {
    case NO_SHARED_FILES:
        FreeMessage(message_from_server);
        FreeMessage(message_for_server);
        return;
        break;
    case FOUND_SHARED_FILES:
        if(CreateSharedFilesDirectory() != -1)
            GetSharedFIles(serverfd, atoi(message_from_server->vars[0]), token);
        break;
    }
}

/************************************************
* INPUT: NULL;                                  *
* OUTPUT: true is successfull, false if failed; *
* Create shared folder.                         *
*************************************************/
int CreateSharedFilesDirectory()
{
    char dir[] = "shared_files\0";
    int ret = -1;
    struct stat st = {0};
    if(stat(dir, &st) == -1)
        ret = mkdir(dir, 0700);
    else    
        return 0;
    return ret;
}
/************************************************************************
* INPUT: server socket, number of files, token of connection;           *
* OUTPUT: NULL;                                                         *
* Get the shared files from server and store them in the shared folder  *
*************************************************************************/
void GetSharedFIles(int serverfd, int num_of_files, TOKEN token)
{
    MESSAGE msg_form_server, msg_for_server;
    FILE* file;
    int file_size;
    char *dir_file;
    for(int i = 0; i < num_of_files; i++)
    {
        msg_form_server = ReadMessage(serverfd);
        if(msg_form_server->code != FILE_NOT_EXIST)
        {
            dir_file = CutDirectoryAndSetLocalDirectory(msg_form_server->vars[0]);
            file_size = atoi(msg_form_server->vars[1]);
            file = CreateFile(dir_file);            
            if(file)
            {
                msg_for_server = Message(CAN_START_UPLOAD_OF_FILE, NULL, token);
                SendMessage(msg_for_server, serverfd);
                if(RecevAndWriteFile(file, file_size, serverfd) == 0)
                {
                    FreeMessage(msg_for_server);
                    msg_for_server = Message(GOT_FILE_SUCCESSFULLY, NULL, token);
                } 
                else
                {
                    FreeMessage(msg_for_server);
                    msg_for_server = Message(FILE_UPLOAD_FAILED, NULL, token);
                }                   
                fclose(file);
            }
            else
                msg_for_server = Message(FAILED_TO_CREATE_FILE_ON_SERVER, NULL, token);
            free(dir_file);

            SendMessage(msg_for_server, serverfd);
            FreeMessage(msg_for_server);
        }
        FreeMessage(msg_form_server);
    }
    msg_form_server = ReadMessage(serverfd);
    if(msg_form_server->code != FINISHED_SHARED_FILES_DOWNLOAD)
        perror("Share files download failed!!!");
    FreeMessage(msg_form_server);
}


char* CutDirectoryAndSetLocalDirectory(char* main_dir)
{
    int ret_len;
    int len_file_name;
    char* ret, local_dir[255];    
    char *temp, *temp1;

    temp = malloc(strlen(main_dir) + SIZE_OF_END_STRING);
    temp1 = temp;
    strcpy(temp, main_dir);
    while(strchr(temp, '/'))
    {
        temp = strchr(temp, '/') + 1;
    }
    len_file_name = strchr(temp, '\0') - temp + 1;

    bzero(local_dir, 255);
    strcat(local_dir, "shared_files/");
    strncat(local_dir, temp, len_file_name);

    ret_len = strlen(local_dir) + 1;
    ret = malloc(ret_len);
    bzero(ret, ret_len);
    strcat(ret, local_dir);
    strcat(ret, "\0");
    free(temp1);
    return ret;
}

FILE* CreateFile(char* file_name)
{
    FILE* ret;
    ret = fopen(file_name, "w");
    if(ret == -1)
        perror("File creation");
    return ret;
}

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
#include "ManageFileUploading.h"
#include "CommunicationProtocol.h"
#include "ProtocolDefinitions.h"
#include "Token.h"
#include "MessageHandle.h"

pid_t child = -1;

int StartUploadingFiles(char *dir, int serverfd, TOKEN token)
{
    LSNODE files = NULL;
    DIR *directory;
    
    directory = opendir(dir);
    MapAllFilesInDirectory(dir, &files, directory);
    SendFilesToServer(dir, files, serverfd, token);
    FreeLSNODE(files);
}

void MapAllFilesInDirectory(char *dir, LSNODE* files, DIR *parent)
{
    DIR *sub_directory;
    struct dirent *file;
    char* sub_dir;
    LSNODE node;
    
    if(parent)
    {
        file = readdir(parent);
        while(file)
        {
            if(strncmp(".", file->d_name, 1))
            {
                node = InsertSafeToLsNode(files, dir, file->d_name, file->d_type);
                if(file->d_type == DT_DIR)
                {
                    sub_dir = ParseTwoPaths(dir, file->d_name);
                    sub_directory = opendir(sub_dir);
                    MapAllFilesInDirectory(sub_dir, &node, sub_directory);
                    free(sub_dir);
                    closedir(sub_directory);
                }
            }
            file = readdir(parent);
        }   
    }
    else
    {
        perror("Directory");
    }
}

LSNODE InsertSafeToLsNode(LSNODE* head, char *dir, char *file_name, char type)
{ 
    LSNODE temp = (*head);
    int len_file = strlen(file_name);
    struct stat file_stat;
    char *full_path = ParseTwoPaths(dir, file_name);

    if(!*head)
    {
        *head = malloc(sizeof(struct FilesInDirectoryNode));
        (*head)->file_name = malloc(len_file + SIZE_OF_END_STRING);
        strncpy((*head)->file_name, file_name, len_file);
        (*head)->file_name[len_file] = '\0';
        (*head)->type = type;
        (*head)->next = NULL;
        (*head)->files_in_dir = NULL;
        stat(full_path, &file_stat);
        (*head)->size = file_stat.st_size;
        return (*head);
    }
    if((*head)->type != DT_DIR)
    {
        while(temp->next)
            temp = temp->next;
        temp->next = malloc(sizeof(struct FilesInDirectoryNode));    
        temp = temp->next;
    }
    else
    { 
        if((*head)->files_in_dir)
        {
            temp = (*head)->files_in_dir;
            while(temp->next)
                temp = temp->next;
            temp->next = malloc(sizeof(struct FilesInDirectoryNode));    
            temp = temp->next;
        }
        else
        {
            temp->files_in_dir = malloc(sizeof(struct FilesInDirectoryNode));    
            temp = temp->files_in_dir;
        }        
        
    }
    temp->file_name = malloc(len_file + SIZE_OF_END_STRING);
    strncpy(temp->file_name, file_name, len_file);
    temp->file_name[len_file] = '\0';
    temp->type = type;
    temp->files_in_dir = NULL;
    temp->next = NULL;
    if(type != DT_DIR)
    {
        stat(full_path, &file_stat);
        temp->size = file_stat.st_size;
    }
    else
        temp->size = 0;

    free(full_path);
    return temp;
}

void SendFilesToServer(char *dir, LSNODE files, int serverfd, TOKEN token)
{
    int file;
    struct stat file_stat;
    LSNODE temp = files;
    MESSAGE msg_for_server, msg_from_server;
    char *file_full_path;    
    char file_size[256];
    char *argv;
    

    while(temp)
    {
        file_full_path = ParseTwoPaths(dir, temp->file_name);
        
        if(temp->type != DT_DIR)
        { 
            file = open(file_full_path, O_RDONLY);
            if(file == -1)
                perror("File: ");        
            if(fstat(file, &file_stat) < 0)
                perror("File fstat: ");
            sprintf(file_size, "%d", file_stat.st_size);   

            argv = ParseTwoArgToString(file_full_path, file_size);
            msg_for_server = Message(UPLOAD_FILE, argv, token), msg_from_server;
            if(SendMessage(msg_for_server, serverfd))
            {
                msg_from_server = ReadMessage(serverfd);
                UploadFileToServer(serverfd, file, token, file_full_path, file_stat, msg_from_server);
            }
            FreeMessage(msg_for_server);    
            
            close(file);
            free(argv);
        }
        else
        {
            msg_for_server = Message(UPLOAD_DIRECTORY, file_full_path, token);
            if(SendMessage(msg_for_server, serverfd))
            {
                msg_from_server = ReadMessage(serverfd);
                switch (msg_from_server->code)
                {
                case MKDIR_SUCCESSFULLY:
                    SendFilesToServer(file_full_path, temp->files_in_dir, serverfd, token);
                    break;
                
                case MKDIR_FAILED:
                    perror("Directory cant be set on server!!");
                    return;
                    break;
                }  
                FreeMessage(msg_from_server);              
            }
            FreeMessage(msg_for_server);            
        }  
        temp = temp->next; 
        free(file_full_path);
    }
}


void UploadFileToServerHandle(MESSAGE msg_form_server, char *file_path, int serverfd, TOKEN token)
{
    int file;
    struct stat file_stat;
    char *argv, *file_full_path;

    file_full_path = malloc(strlen(file_path) + SIZE_OF_END_STRING);
    strcpy(file_full_path, file_path);
    strcat(file_full_path, "\0");

    file = open(file_full_path, O_RDONLY);
    if(file == -1)
        perror("File: ");
    else if(fstat(file, &file_stat) < 0)
        perror("File stat: ");

    UploadFileToServer(serverfd, file, token, file_full_path, file_stat, msg_form_server);
    close(file);
    
}

void UploadFileToServer(int serverfd, int file, TOKEN token, char* file_full_path, struct stat file_stat, MESSAGE msg_from_server)
{    
    MESSAGE msg_stat_file;
    switch (msg_from_server->code)
    {
    case CAN_START_UPLOAD_OF_FILE: 
        SendFile(file, serverfd, file_stat); 
        FreeMessage(msg_from_server);
        msg_stat_file = ReadMessage(serverfd);
        if(msg_stat_file->code != GOT_FILE_SUCCESSFULLY)
            perror("File upload went wrong!!");
        else
        {
            printf("File: %s Was uploaded successfully!\n", file_full_path);
            FreeMessage(msg_stat_file);
        }
    break;                
    default:
        perror("Server cant get file!!");
        break;
    }        
}
void SetAutoSyncProccess(CLIENT client, char *dir, TOKEN token)
{
    LSNODE ls1 = NULL, ls2 = NULL, files_to_upload = NULL;
    DIR *directory;   
    child = fork();
    if(child == 0)
    {
        directory = opendir(dir);
        MapAllFilesInDirectory(dir, &ls1, directory);
        closedir(directory);
        while (1)// get signal to stop;
        { 
            directory = opendir(dir);
            MapAllFilesInDirectory(dir, &ls2, directory);
            closedir(directory);
            if(ComperTwoLSNODEs(ls1, ls2))
            {
                GetChangedFiles(ls1, ls2, &files_to_upload, dir);
                client->in_use = 1;
                SendFilesToServer(dir, files_to_upload, client->sockfd, token);
                FreeLSNODE(ls1);
                ls1 = NULL;
                directory = opendir(dir);
                MapAllFilesInDirectory(dir, &ls1, directory);
                closedir(directory);
                client->in_use = 0;
            }            
            FreeLSNODE(ls2);
            FreeLSNODE(files_to_upload);
            ls2 = NULL;
            files_to_upload = NULL;
            sleep(120);
        }
        FreeLSNODE(ls1);            
    }
    
}

int ComperTwoLSNODEs(LSNODE l1, LSNODE l2)
{
    while(l1 && l2)
    {
        if(!strcmp(l1->file_name, l2->file_name) && l1->type == l2->type && l1->size == l2->size)
        {
            l1 = l1->next;
            l2 = l2->next;
        }
        else
            return 1;
    }
    return 0;
}

void GetChangedFiles(LSNODE l1, LSNODE l2, LSNODE* files_changed, char *dir)
{
    while(l1 && l2)
    {
        if(strcmp(l1->file_name, l2->file_name) || l1->type != l2->type || l1->size != l2->size)
            InsertSafeToLsNode(files_changed, dir, l2->file_name, l2->type); 
        if(!l1->files_in_dir && !l2->files_in_dir)
        {
            l1 = l1->next;
            l2 = l2->next;
        }
        else
        {
            l1 = l1->files_in_dir;
            l2 = l2->files_in_dir;
        }
    }
}

char* ParseTwoPaths(char* p1,char* p2)
{
    int len_dir = strlen(p1) + SIZE_OF_DELIMITER;
    int len_file_name = strlen(p2) + SIZE_OF_END_STRING;
    char *ret = malloc(len_dir + len_file_name);
    bzero(ret, len_file_name + len_dir);
    strcat(ret, p1);
    strcat(ret, "/");
    strcat(ret, p2);
    strcat(ret, "\0");
    return ret;
}

void FreeLSNODE(LSNODE head)
{
    LSNODE temp;
    while(head)
    {
        temp = head;
        if(!head->files_in_dir)    
        {     
            head = head->next;
            free(temp->file_name);
            free(temp); 
            temp = NULL;
        }
        else
        {
            FreeLSNODE(head->files_in_dir);
            head->files_in_dir = NULL;
        }        
    }
}

void StopAutoSyncProccess()
{
    kill(child, SIGKILL);
}
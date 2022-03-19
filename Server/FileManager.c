#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "FileManager.h"
#include "ProtocolDefinitions.h"
#include "DataBaseManager.h"

int MakeDirectoryOnServerForUser(char *user_id)
{
    char *dir;
    int len_dir, ret = -1;
    struct stat st = {0};
    len_dir = strlen("server_storage/") + strlen(user_id) + SIZE_OF_END_STRING;
    dir = malloc(len_dir);
    bzero(dir, len_dir);
    strcat(dir, "server_storage/");
    strcat(dir, user_id);
    dir[len_dir - 1] = '\0';
    if(stat(dir, &st) == -1)
    {        
        ret = mkdir(dir, 0700);
        free(dir);
    }
    else
    {
        free(dir);
        return 0;
    }
    return ret;
}

char* CutDirectoryAndSetLocalDirectory(char* main_dir, char* to_cut, char *user_id)
{
    int len_main_dir, len_to_cut, ret_len;
    char* ret, local_dir[255];    
    bzero(local_dir, 255);
    strcat(local_dir, "server_storage/");
    strcat(local_dir, user_id);
    
    len_main_dir = strlen(main_dir);
    len_to_cut = strlen(to_cut);

    ret_len = len_to_cut - len_main_dir + strlen(local_dir) + SIZE_OF_END_STRING;
    ret = malloc(ret_len);
    bzero(ret, ret_len);
    strcat(ret, local_dir);
    memcpy(ret + strlen(local_dir), to_cut + len_main_dir, len_to_cut - len_main_dir);
    strcat(ret, "\0");

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

int MakeDirectory(char *dir)
{
    int ret = -1;
    struct stat st = {0};
    if(stat(dir, &st) == -1)
        ret = mkdir(dir, 0700);
    else
    
        return 0;
    return ret;
}

int CheckIfFileExistInServer(MYSQL_MANAGER* db_manager, char *file_path, char *user_id)
{
    char *file_path_in_server = CutDirectoryAndSetLocalDirectory(GetDirectoryFromDataBase(db_manager, user_id), file_path, user_id);
    FILE* file;
    file = fopen(file_path_in_server, "w");
    if(file == -1)    
    {
        fclose(file);
        return 0;
    }
    return 1;
}
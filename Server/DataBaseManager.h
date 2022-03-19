#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

struct MysqlManager
{
    MYSQL* db;
    int in_use;
};

typedef struct MysqlManager MYSQL_MANAGER;
MYSQL_MANAGER* ConnectToDataBase();
int SetUpDataBase(MYSQL_MANAGER* db_manager);
int CreateDataBaseAndTables(MYSQL_MANAGER* db_manager);

int CheckUserIsRegistered(MYSQL_MANAGER* db_manager, char *email);
int InsertNewUser(MYSQL_MANAGER* db_manager, char *email, char *password);
int CheckUserCredentials(MYSQL_MANAGER* db_manager, char *email, char *password);
int GetUserId(MYSQL_MANAGER* db_manager, char *email);

int ChangeDirectoryInDataBase(MYSQL_MANAGER* db_manager, char *dir, char *user_id);
char* GetDirectoryFromDataBase(MYSQL_MANAGER* db_manager, char* user_id);

int GetListOfSharedFiles(MYSQL_MANAGER* db_manager, char* user_id, char ***array_of_files_location);

int SetFileToBeShared(MYSQL_MANAGER* db_manager, char* file_path, char* email, char* user_id);

void PrintErrorDataBase(MYSQL_MANAGER* db_manager);

#endif
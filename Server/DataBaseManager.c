#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include "DataBaseManager.h"
#include "ProtocolDefinitions.h"
#include "FileManager.h"

MYSQL_MANAGER* ConnectToDataBase()
{
    MYSQL_MANAGER* db_manager;
    db_manager = malloc(sizeof(MYSQL_MANAGER));
    db_manager->db = mysql_init(NULL);
    if(db_manager->db == NULL)
    {
        perror(strerror(mysql_errno(db_manager->db)));
        return NULL;
    }
    
    if(mysql_real_connect(db_manager->db, "localhost", "root", "admin", NULL, 0, NULL, 0) == NULL)
    {
        perror(strerror(mysql_errno(db_manager->db)));
        mysql_close(db_manager->db);
        return NULL;
    }

    db_manager->in_use = false;

    printf("Data Base connection is up and running...\n");
    return db_manager;
}

int SetUpDataBase(MYSQL_MANAGER* db_manager)
{
    if(mysql_query(db_manager->db, "USE bm_drive"))
        return CreateDataBaseAndTables(db_manager);        
    return 0;
}

int CreateDataBaseAndTables(MYSQL_MANAGER* db_manager)
{
    // Create main data base
    if(mysql_query(db_manager->db, "CREATE DATABASE bm_drive"))
    {
        PrintErrorDataBase(db_manager->db);
        return EXIT_FAILURE;
    }
    // Use main data base
    if(mysql_query(db_manager->db, "USE bm_drive"))
    {
        PrintErrorDataBase(db_manager->db);
        return EXIT_FAILURE;
    }
    // Create users data base table
    if(mysql_query(db_manager->db, "CREATE TABLE Users (Id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(Id), Email TEXT, Password TEXT)"))
    {
        PrintErrorDataBase(db_manager->db);
        return EXIT_FAILURE;
    }
    // Create stored file directory of users data base table
    if(mysql_query(db_manager->db, "CREATE TABLE Directories (Id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(Id), UserId INT NOT NULL, PrivateDirectory TEXT, SharedDirectory TEXT)"))
    {
        PrintErrorDataBase(db_manager->db);
        return EXIT_FAILURE;
    }
    // Create shared files data base table
    if(mysql_query(db_manager->db, "CREATE TABLE SharedFiles (Id INT NOT NULL AUTO_INCREMENT, OwnerId INT NOT NULL, OtherUserId INT NOT NULL, FilePath TEXT NOT NULL , PRIMARY KEY (Id))"))
    {
        PrintErrorDataBase(db_manager->db);
        return EXIT_FAILURE;
    }
    printf("Data base and tables were created...\n");
    return 0;
}

int InsertNewUser(MYSQL_MANAGER* db_manager, char *email, char *password)
{
    char query[255] = "INSERT INTO `Users` (`Email`, `Password`) VALUES ('";
    char num[4];
    strncat(query, email, strlen(email));
    strcat(query, "', '");
    strncat(query, password, strlen(password));
    strcat(query, "') ");
    if(mysql_query(db_manager->db,  query))
    {
        PrintErrorDataBase(db_manager);
        return EXIT_FAILURE;
    }
    bzero(query, 255);
    memcpy(query, "INSERT INTO `Directories` (`UserId`) VALUES (", strlen("INSERT INTO `Directories` (`UserId`) VALUES ("));
    sprintf(num, "%d", GetUserId(db_manager, email));
    strncat(query, num, 3);
    strcat(query, ")");
    if(mysql_query(db_manager->db,  query))
    {
        PrintErrorDataBase(db_manager);
        return EXIT_FAILURE;
    }
    return 0;
}

int CheckUserIsRegistered(MYSQL_MANAGER* db_manager, char *email)
{
    MYSQL_RES *result;
    char query[255] = "SELECT `Id` FROM `Users` WHERE `Email` = '";
    strncat(query, email, strlen(email));
    strcat(query, "'");
    if(mysql_query(db_manager->db,  query))
    {
        PrintErrorDataBase(db_manager);
        return EXIT_FAILURE;
    }
    result = mysql_store_result(db_manager->db);
    if(mysql_num_rows(result))
    {
        mysql_free_result(result);
        return 1;
    }
    mysql_free_result(result);
    return 0;
}

int CheckUserCredentials(MYSQL_MANAGER* db_manager, char *email, char *password)
{
    MYSQL_RES *result;
    char query[255] = "SELECT * FROM `Users` WHERE `Email` = '";
    strncat(query, email, strlen(email));
    strcat(query, "' AND `Password` = '");
    strncat(query, password, strlen(password));
    strcat(query, "' ");
    if(mysql_query(db_manager->db,  query))
    {
        PrintErrorDataBase(db_manager);
        return EXIT_FAILURE;
    }
    result = mysql_store_result(db_manager->db);
    if(mysql_num_rows(result))
    {
        mysql_free_result(result);
        return 1;
    }
    mysql_free_result(result);
    return 0;
}

int GetUserId(MYSQL_MANAGER* db_manager, char *email)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int id;
    char query[255] = "SELECT * FROM `Users` WHERE `Email` = '";
    strncat(query, email, strlen(email));
    strcat(query, "' ");
    if(mysql_query(db_manager->db,  query))
    {
        PrintErrorDataBase(db_manager);
        return EXIT_FAILURE;
    }
    result = mysql_store_result(db_manager->db);
    if(mysql_num_rows(result))
    {
        row = mysql_fetch_row(result);
        id = atoi(row[0]);
        mysql_free_result(result);        
        return id;
    }
    mysql_free_result(result);
    return 0;
}

int ChangeDirectoryInDataBase(MYSQL_MANAGER* db_manager, char *dir, char *user_id)
{
    char query[255] = "UPDATE `Directories` SET `PrivateDirectory` = '";
    strcat(query, dir);
    strcat(query, "' WHERE `UserId` = ");
    strcat(query, user_id);
    strcat(query, "\0");
    if(mysql_query(db_manager->db,  query))
    {
        PrintErrorDataBase(db_manager);
        return EXIT_FAILURE;
    }
    if(mysql_affected_rows(db_manager->db))
    {
        return 1;
    }
    return 0;
}

char* GetDirectoryFromDataBase(MYSQL_MANAGER* db_manager, char* user_id)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    char* dir;    
    char query[255] = "SELECT * FROM `Directories` WHERE `UserId` = ";
    strncat(query, user_id, strlen(user_id));
    if(mysql_query(db_manager->db,  query))
    {
        PrintErrorDataBase(db_manager);
        return EXIT_FAILURE;
    }
    result = mysql_store_result(db_manager->db);
    if(mysql_num_rows(result))
    {
        row = mysql_fetch_row(result);
        dir = malloc(strlen(row[2]) + SIZE_OF_END_STRING);
        strncpy(dir, row[2], strlen(row[2]));
        dir[strlen(row[2])] = '\0';
        mysql_free_result(result);        
        return dir;
    }
    mysql_free_result(result);
    return NULL;
}

int SetFileToBeShared(MYSQL_MANAGER* db_manager, char* file_path, char* email, char* user_id)
{
    char query[255];
    char other_user_id[4];
    char *storage_dir;
    char *dir_of_file_in_server;
    // Get other user id by email
    sprintf(other_user_id, "%d", GetUserId(db_manager, email));

    // Get starting path of shared space
    storage_dir = GetDirectoryFromDataBase(db_manager, user_id);
    if(storage_dir)
    {
        // Cut starting path of shared space from file path
        // Add user stored location on server to file path
        dir_of_file_in_server = CutDirectoryAndSetLocalDirectory(storage_dir, file_path, user_id);
        // Insert into data base shared file table file path on server
        bzero(query, 255);
        strcpy(query, "INSERT INTO `SharedFiles` (`OwnerId`, `OtherUserId`, `FilePath`) VALUES (\0");
        strcat(query, user_id);
        strcat(query, ", \0");
        strcat(query, other_user_id);
        strcat(query, ", '\0");
        strcat(query, dir_of_file_in_server);
        strcat(query, "') \0");
        if(mysql_query(db_manager->db, query))
        {
            PrintErrorDataBase(db_manager);
            return EXIT_FAILURE;
        }
        return 0;
    }
}

int GetListOfSharedFiles(MYSQL_MANAGER* db_manager, char* user_id, char ***array_of_files_location)
{
    char query[255];
    int num_rows;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int i = 0;
    bzero(query, 255);
    //Get all rows from SharedFiles with user id
    strcpy(query, "SELECT * FROM `SharedFiles` WHERE `OtherUserId` = '\0");
    strcat(query, user_id);
    strcat(query, "'");
    if(mysql_query(db_manager->db, query))
    {
        PrintErrorDataBase(db_manager);
        return -1;
    }
    result = mysql_store_result(db_manager->db);
    //Count all rows
    num_rows = mysql_num_rows(result);    
    //Add to 2d array files location
    if(num_rows)
    {
        (*array_of_files_location) = malloc(sizeof(char*) * num_rows);
        while(row = mysql_fetch_row(result))
        {
            (*array_of_files_location)[i] = malloc(strlen(row[3]) + SIZE_OF_END_STRING);
            strcpy((*array_of_files_location)[i], row[3]);
            strcat((*array_of_files_location)[i], "\0");
            i++;
        }
    }
    mysql_free_result(result);
    return num_rows;
}

void PrintErrorDataBase(MYSQL_MANAGER* db_manager)    
{
    perror(strerror(mysql_errno(db_manager->db)));
    mysql_close(db_manager->db);
}
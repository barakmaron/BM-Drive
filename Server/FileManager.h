#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include "DataBaseManager.h"

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

int MakeDirectoryOnServerForUser(char *user_id);
char* CutDirectoryAndSetLocalDirectory(char* main_dir, char* to_cut, char *user_id);
FILE* CreateFile(char* file_name);
int MakeDirectory(char *dir);
int CheckIfFileExistInServer(MYSQL_MANAGER* db_manager, char *file_path, char *user_id);

#endif
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

#ifndef MANAGEFILESHARING_H
#define MANAGEFILESHARING_H

void GetFilesSharedWithUser(int serverfd, TOKEN token);
int CreateSharedFilesDirectory();
void GetSharedFIles(int serverfd, int num_of_files, TOKEN token);
char* CutDirectoryAndSetLocalDirectory(char* main_dir);
FILE* CreateFile(char* file_name);

#endif
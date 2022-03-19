#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <dirent.h>
#include "CommunicationProtocol.h"
#include "ProtocolDefinitions.h"
#include "Token.h"
#include "MessageHandle.h"

#ifndef MANAGEDILEUPLOADING_H
#define MANAGEDILEUPLOADING_H
struct FilesInDirectoryNode
{
    char *file_name;
    char type;
    off_t size;
    struct FilesInDirectoryNode* files_in_dir;
    struct FilesInDirectoryNode* next;
};

typedef struct FilesInDirectoryNode* LSNODE;

int StartUploadingFiles(char *dir, int serverfd, TOKEN token);
LSNODE InsertSafeToLsNode(LSNODE* head, char *dir,char *file_name, char type);
LSNODE SetLinkListOfFilesInDirectory(FILE *ls_file_output);
void MapAllFilesInDirectory(char *dir, LSNODE *files, DIR *parent);
void SendFilesToServer(char *dir, LSNODE files, int serverfd, TOKEN token);
void UploadFileToServerHandle(MESSAGE msg_form_server, char *file_path, int serverfd, TOKEN token);
void UploadFileToServer(int serverfd, int file, TOKEN token, char* file_full_path, struct stat file_stat, MESSAGE msg_from_server);
void SetAutoSyncProccess(CLIENT client, char *dir, TOKEN token);
void StopAutoSyncProccess();
int ComperTwoLSNODEs(LSNODE l1, LSNODE l2);
void GetChangedFiles(LSNODE l1, LSNODE l2, LSNODE* files_changed, char *dir);

char* ParseTwoPaths(char* p1,char* p2);
void FreeLSNODE(LSNODE head);

#endif
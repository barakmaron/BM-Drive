#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <mysql/mysql.h>
#include "DataBaseManager.h"
#include "ProtocolDefinitions.h"

#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

void SendConnectionStarted(int clientfd);
void RegisterNewUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void LoginUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void ChangeDirectory(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void UploadAllFiles(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void UploadDirectory(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void LogoutUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void AutoSync(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void StopAutoSync(MESSAGE msg, int clientfd);
void ShareFileWithUser(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);
void GetSharedFiles(MESSAGE msg, int clientfd, MYSQL_MANAGER* db_manager);

#endif
#include <stdio.h>
#include <stdlib.h>
#include "Client.h"
#include "CommunicationProtocol.h"
#include "Token.h"
#ifndef MESSAGEHANDLE_H
#define MESSAGEHANDLE_H

void HandleMessageFromServer(CLIENT client, MESSAGE msg_sent_to_server, TOKEN *token);
void RegisterHandle(MESSAGE msg_from_server, MESSAGE msg_sent_to_server);
TOKEN LoginHandle(int serverfd, MESSAGE msg_from_server, MESSAGE msg_sent_to_server);
void ChangeDirectoryHandle(MESSAGE msg_from_server, MESSAGE msg_sent_to_server);
void UploadAllFileHandle(int serverfd, MESSAGE msg_from_server, MESSAGE msg_sent_to_server);
void UploadFileHandle(int serverfd, MESSAGE msg_from_server, MESSAGE msg_sent_to_server);
void LogoutHandle(CLIENT client, MESSAGE msg_from_server, TOKEN *token);
void SetAutoSyncHandle(CLIENT client, MESSAGE msg_from_server);
void StopAutoSyncHandle(int clientfd, MESSAGE msg_from_server);
void ShareFileHandle(int clientfd, MESSAGE msg_from_server);

#endif

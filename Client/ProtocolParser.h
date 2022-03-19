#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <error.h>
#include "CommunicationProtocol.h"
#include "Token.h"

#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

MESSAGE RegisterUser(int clientfd);
MESSAGE LoginUser(int clientfd);
MESSAGE SetDirectory(int clientfd, TOKEN token);
MESSAGE UploadAllFiles(int clientfd, TOKEN token);
MESSAGE UploadFile(int clientfd, TOKEN token);
MESSAGE LogoutUser(int clientfd, TOKEN token);
MESSAGE SetAutoSync(int clientfd, TOKEN token);
MESSAGE StopAutoSync(int clientfd, TOKEN token);
MESSAGE ShareFileWithOtherUser(int clientfd, TOKEN token);

#endif
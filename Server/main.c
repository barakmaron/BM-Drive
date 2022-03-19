#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "Server.h"

int main(int argc, char **argv)
{
    struct Server *s = malloc(sizeof(struct Server));
    ServerConstruct(s);
    free(s);
    return 0;
}
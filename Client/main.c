#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include "Client.h"

int main(int argc, char *argv[])
{
    CLIENT c;
    c = malloc(sizeof(struct Client));
    ClientConstruct(c, argv[1]);

    if(!StartConnectionByProtocol(c))
    {
        perror("Start connection by protocol");
        close(c->sockfd);
        free(c);
        exit(EXIT_FAILURE);
    }

    printf("Connection is established\n");
    printf("Enter 'h' to get the list of commands.\n");

    HandleCommandsFromUser(c);

    close(c->sockfd);
    free(c);
    return 0;
}
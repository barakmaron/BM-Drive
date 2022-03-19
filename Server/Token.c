#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <time.h>
#include "ProtocolDefinitions.h"
#include "Token.h"

TOKEN CreateToken(int id)
{
    TOKEN ret;
    ret = malloc(sizeof(struct Token));
    ret->token_string = calloc(4, 4);
    sprintf(ret->token_string, "%d", id);
    ret->token_string[3] = '\0';
    time(&ret->time);
    return ret;
}

int ValidateToken(TOKEN token)
{

}

TOKEN CreateCopyToken(TOKEN token_to_copy)
{
    TOKEN ret;
    int len;
    if(token_to_copy)
    {
        len = strlen(token_to_copy->token_string) + SIZE_OF_END_STRING;
        ret = malloc(sizeof(struct Token));
        ret->token_string = malloc(len);
        strncpy(ret->token_string, token_to_copy->token_string, len);
        ret->time = token_to_copy->time;
    }
    else
        ret = NULL;
    return ret;
}

TOKEN CreateTokenFromString(char *token)
{
    TOKEN ret;   
    ret = malloc(sizeof(struct Token));
    ret->token_string = calloc(4, 4);
    strncpy(ret->token_string, token, 3);
    ret->token_string[3] = '\0';
    ret->time = atoi(strchr(token, '&') + 1);
    return ret;
}

char* ToStringToken(TOKEN token)
{
    unsigned char *ret;    
    unsigned char num[sizeof(time_t) * 8];   
    int len = SIZE_OF_PROTOCOL_CODE_IN_STRING + SIZE_OF_DELIMITER + sizeof(time_t) * 8 + SIZE_OF_END_STRING; 
    if(token)
    {
        ret = calloc(len, len);
        strncpy(ret, token->token_string, 3);
        strcat(ret, "&");
        sprintf(num, "%ld", token->time);
        strcat(ret, num);
        strcat(ret, "\0");
    }
    else
        return NULL;
    return ret;
}

void FreeToken(TOKEN token)
{
    if(token)
    {
        free(token->token_string);
        free(token);
    }
}
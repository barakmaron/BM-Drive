#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <time.h>

#ifndef TOKEN_H
#define TOKEN_H

struct Token
{
    char *token_string;
    time_t time;
};

typedef struct Token* TOKEN;

TOKEN CreateToken(int id);
TOKEN CreateTokenFromString(char *token);
TOKEN CreateCopyToken(TOKEN token_to_copy);
char* ToStringToken(TOKEN token);
int ValidateToken(TOKEN token);
void FreeToken(TOKEN token);

#endif
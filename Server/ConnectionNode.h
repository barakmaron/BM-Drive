#ifndef CONNECTIONNODE_H
#define CONNECTIONNODE_H

#include <stdio.h>

struct ConnectionNode
{
    int value;    
    struct ConnectionNode* next;
};

void InsertConnectionNode(struct ConnectionNode* head, int value);
struct ConnectionNode* FindConnectionNode(struct ConnectionNode* head, int value);
void FreeConnectionNode(struct ConnectionNode* head);

#endif
